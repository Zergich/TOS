
#include <print.h>
#include <stdint.h>

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

// Описание регистра IDTR для lidt
struct {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed)) idtr;

// Установка одной записи в IDT
void set_idt_gate(int n, void (*handler)(void)) {
  uint64_t handler_addr = (uint64_t)handler;

  idt[n].offset_low = handler_addr & 0xFFFF;
  idt[n].selector = 0x08;  // обычно это селектор кода в GDT
  idt[n].ist = 0;          // без использования IST
  idt[n].type_attr = 0x8E; // прерывание, присутствует, DPL=0
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

// Пример обработчика деления на ноль
// __attribute__((interrupt)) дает правильный calling convention
// void divide_by_zero_handler(struct interrupt_frame *frame)
__attribute__((interrupt)) void
divide_by_zero_handler(struct interrupt_frame *frame) {
  // Тут можно обработать ошибку, вывести сообщение, логгировать и т.п.
  // Для примера — бесконечный цикл, чтобы остановиться
  printf("%u", frame->rflags);
  print("pede");
  asm volatile("hlt");
}

// Инициализация IDT – назначаем обработчики
void idt_init() {
  // Обнуляем таблицу
  for (int i = 0; i < 256; i++) {
    set_idt_gate(i, 0); // нулевой указатель - может быть заменён на заглушку
  }

  // Устанавливаем обработчик деления на ноль в вектор 0
  set_idt_gate(0, (void *)divide_by_zero_handler);

  // Загружаем таблицу
  load_idt();
}
