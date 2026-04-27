
#include <System/interrupts.h>
#include <System/io.h>
#include <System/keyboard.h>
#include <System/time.h>
#include <VGA/print.h>
#include <VGA/rsod.h>
#include <datastruct.h>
#include <stdint.h>

#pragma GCC target("general-regs-only") // запрет на поддержку SSE/AVX
                                        // выключение регистров XMM/YMM

extern RoundBufferObgect RoundBuff;
// Структура, описывающая фрейм прерывания (x86_64)
struct interrupt_frame {
  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
} __attribute__((packed));

// Структура записи в IDT (16 байт)
struct idt_entry {
  uint16_t offset_low;  // биты 0..15 адреса обработчика
  uint16_t selector;    // селектор сегмента кода (например, 0x08)
  uint8_t ist;          // Interrupt Stack Table (обычно 0)
  uint8_t type_attr;    // атрибуты типа (0x8E - прерывание)
  uint16_t offset_mid;  // биты 16..31 адреса обработчика
  uint32_t offset_high; // биты 32..63 адреса обработчика
  uint32_t zero;        // нули
} __attribute__((packed));

// IDT — массив из 256 записей
struct idt_entry idt[256];

extern u32 *pede;

// Описание регистра IDTR для lidt
struct {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed)) idtr;

// Установка одной записи в IDT
void set_idt_gate(int n, void (*handler)(void)) {
  uint64_t handler_addr = (uint64_t)handler;

  uint16_t current_cs;
  asm volatile("mov %%cs, %0" : "=r"(current_cs));

  idt[n].offset_low = handler_addr & 0xFFFF;
  idt[n].selector = current_cs; // обычно это селектор кода в GDT
  idt[n].ist = 0;               // без использования IST
  idt[n].type_attr = 0x8E;      // прерывание, присутствует, DPL=0
  idt[n].offset_mid = (handler_addr >> 16) & 0xFFFF;
  idt[n].offset_high = (handler_addr >> 32) & 0xFFFFFFFF;
  idt[n].zero = 0;
}

// Загрузить IDT регистр
void load_idt() {
  idtr.limit = sizeof(idt) - 1;
  idtr.base = (uint64_t)&idt;
  asm volatile("lidt %0" : : "m"(idtr));
}

// зарезервированные ошибки
__attribute__((interrupt)) void
divide_by_zero_handler(struct interrupt_frame *frame) {
  DivideZero();
  asm volatile("hlt");
}
__attribute__((interrupt)) void
page_fault_handler(struct interrupt_frame *frame) {
  MappingError();
  asm volatile("hlt");
}
__attribute__((interrupt)) void
invalide_opcode_handler(struct interrupt_frame *frame) {
  OpcodeError();
  asm volatile("hlt");
}
__attribute__((interrupt)) void
double_fault_handler(struct interrupt_frame *frame) {
  OpcodeError();
  asm volatile("hlt");
}
__attribute__((interrupt)) void
general_protection_handler(struct interrupt_frame *frame) {
  GeneralProtectionError();
  asm volatile("hlt");
}

#define KBD_DATA_PORT 0x60
volatile int key_pressed = 0;
volatile u8 last_char = 0;
__attribute__((interrupt)) void
keyboard_handler(struct interrupt_frame *frame) {
  uint8_t scancode = inb(KBD_DATA_PORT);
  u8 c = ReturnCharKeyboard(scancode);
  if (c) {
    last_char = c;
    key_pressed = 1;
    RoundBuff.put(c);
  }
  // Отправляем EOI (End of Interrupt) контроллеру PIC
  outb(PIC1_COMMAND, PIC_EOI);
}

void pic_remap() {
  uint8_t a1, a2;

  a1 = inb(0x21); // Сохрани маски
  a2 = inb(0xA1);

  outb(0x20, 0x11); // Начало инициализации master PIC
  outb(0xA0, 0x11); // Начало инициализации slave PIC
  outb(0x21, 0x20); // Вектор прерывания master = 32
  outb(0xA1, 0x28); // Вектор прерывания slave = 40
  outb(0x21, 0x04); // Настройка связи master-slave
  outb(0xA1, 0x02);
  outb(0x21, 0x01); // Режим 8086
  outb(0xA1, 0x01);
  outb(0x21, a1); // Восстановить маску
  outb(0xA1, a2);
  uint8_t current_mask = inb(0x21);
  // Сбрасываем биты 0 и 1 (0 = enable, 1 = disable)
  // разрешает и таймер (0/32) и клавиатуру (1/33х)
  outb(0x21, current_mask & ~(1 << 1) & ~(1 << 0));
}
__attribute__((interrupt)) void empty_handler(struct interrupt_frame *frame) {
  asm volatile("iretq");
}

extern TimePit Timepit;
volatile u16 PitTicks = 0; // тики и так сбрасываются переполнением
__attribute__((interrupt)) void pit_hendler(struct interrupt_frame *frame) {
  PitTicks++;
  if (PitTicks % 1000 == 0)
    Timepit.PitTimerSecondsUp++;
  Timepit.PitTimerMiliSecondsUp++;
  outb(PIC1_COMMAND, PIC_EOI);
}

// Инициализация IDT – назначаем обработчики
void idt_init() {

  for (int i = 0; i < 256; i++) {
    set_idt_gate(i, (void *)empty_handler); // нулевой указатель - может быть
                                            // заменён на заглушку
  }
  pic_remap();

  // Устанавливаем обработчик деления на ноль в вектор 0
  set_idt_gate(33, (void *)keyboard_handler);
  set_idt_gate(32, (void *)pit_hendler);

  set_idt_gate(0, (void *)divide_by_zero_handler);
  set_idt_gate(6, (void *)invalide_opcode_handler);
  set_idt_gate(8, (void *)double_fault_handler);
  set_idt_gate(13, (void *)general_protection_handler);
  set_idt_gate(14, (void *)page_fault_handler);
  // set_idt_gate(33, (void *)keyboard_handler);

  // Загружаем таблицу
  load_idt();
}
