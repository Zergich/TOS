
#include <ConsoleIO/console.h>
#include <System/rsod.h>
#include <arch/x86_64/interrupts.h>
#include <arch/x86_64/io.h>
#include <drivers/keyboard.h>
#include <libs/datastruct.h>
#include <libs/time.h>
#include <stdint.h>

#include <System/MemoryManager/PMM.h>
#include <System/MemoryManager/VMM.h>

#pragma GCC target("general-regs-only") // запрет на поддержку SSE/AVX
                                        // выключение регистров XMM/YMM

extern RoundBufferObgect RoundBuff;
// Структура, описывающая фрейм прерывания (x86_64)
struct interrupt_frame {
  uint64_t error_code; // Процессор кладет его первым!
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

// Флаги кода ошибки Page Fault (биты error_code)
#define PF_ERROR_PRESENT                                                       \
  (1 << 0) // 0 = страница не существует, 1 = нарушение прав доступа
#define PF_ERROR_WRITE (1 << 1) // 0 = чтение, 1 = попытка записи
#define PF_ERROR_USER (1 << 2)  // 0 = ядро (Ring 0), 1 = пользователь (Ring 3)
#define PF_ERROR_FETCH (1 << 4) // 1 = попытка выполнения кода (NX бит)

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
static inline int is_lazy_allocation_allowed(uint64_t fault_addr, uintptr_t error_code) {
    // Бит 2 в error_code: 1 - ошибка из user-mode, 0 - из kernel-mode
    int is_user = error_code & (1 << 2);

    // 1. ПРАВИЛО: Если ошибка произошла из-за нарушения прав (например, запись в Read-Only),
    // это не ленивая аллокация, это баг защиты!
    if (error_code & (1 << 0)) {
        return 0;
    }

    // 2. ПРАВИЛО: Пользовательское пространство (Lower Half)
    // В будущем здесь будет проверка по структурам процесса: if (vma_find(current_process, fault_addr))
    // Сейчас разрешаем ленивую аллокацию для пользовательской памяти (например, ниже 0x00007FFFFFFFFFFF)
    if (is_user && fault_addr < 0x0000800000000000ULL) {
        return 1;
    }

    // 3. ПРАВИЛО: Пространство ядра (Higher Half)
    // Разрешаем ленивую аллокацию только для региона динамической кучи ядра (Kernel Heap).
    // Замени KERNEL_HEAP_START и KERNEL_HEAP_END на свои реальные границы кучи ядра, когда напишешь kmalloc.
    /*
    extern uint64_t KERNEL_HEAP_START;
    extern uint64_t KERNEL_HEAP_END;
    if (!is_user && fault_addr >= KERNEL_HEAP_START && fault_addr < KERNEL_HEAP_END) {
        return 1;
    }
    */

    return 0; // Для всех остальных адресов ленивая аллокация запрещена
}

__attribute__((interrupt)) void page_fault_handler(struct interrupt_frame *frame, uintptr_t error_code) {
    uint64_t fault_addr;
    // Извлекаем адрес, вызвавший исключение, из управляющего регистра CR2
    __asm__ volatile("mov %%cr2, %0" : "=r"(fault_addr));

    // Выравниваем адрес вниз до ближайшей границы 4-килобайтной страницы
    uint64_t aligned_fault_addr = fault_addr & ~(PAGE_SIZE - 1);

    // Проверяем, имеем ли мы право обработать этот fault лениво
    if (is_lazy_allocation_allowed(fault_addr, error_code)) {
        
        // Запрашиваем физический фрейм у PMM
        uintptr_t phys = pmm_alloc_frame();
        if (phys == 0) {
            // Если физическая память физически кончилась — это системная паника
            Panic("Page Fault Handler: Out of physical memory during lazy allocation!");
        }

        // Определяем текущее адресное пространство.
        // Если упал пользовательский процесс, тут должен быть его space.
        // Пока мы в ядре — берем глобальный kernel_space.
        extern address_space_t kernel_space; 
        
        // Набор флагов для ленивой страницы.
        // Если упал юзер, добавляем флаг PTE_USER, чтобы процессор разрешил ему доступ.
        uint64_t vmm_flags = PTE_PRESENT | PTE_WRITABLE;
        if (error_code & (1 << 2)) {
            vmm_flags |= PTE_USER;
        }

        // Мапируем физический фрейм на виртуальную страницу
        vmm_map_page(&kernel_space, aligned_fault_addr, phys, vmm_flags);

        // Возвращаем управление. Процессор перезапустит упавшую инструкцию.
        return;
    }

    // =========================================================================
    // КРАШ-ДАННЫЕ (Если это честный, необработанный баг ядра или процесса)
    // =========================================================================
  PageError();
    print("\n!!! UNHANDLED PAGE FAULT !!!\n");
    print("  Fault Address:  0x"); printf("%h", fault_addr); print("\n");
    print("  Error Code:     0x"); printf("%h", error_code); print("\n");
    print("  RIP (Code Ptr): 0x"); printf("%h", frame->rip); print("\n");

    // Расшифровка причин
    if (error_code & (1 << 0)) {
        print("  Reason: Access rights violation (Protection Fault)\n");
    } else {
        print("  Reason: Page not present\n");
    }

    if (error_code & (1 << 1)) {
        print("  Operation: WRITE\n");
    } else if (error_code & (1 << 4)) {
        print("  Operation: INSTRUCTION FETCH (Execute)\n");
    } else {
        print("  Operation: READ\n");
    }

    if (error_code & (1 << 2)) {
        print("  Privilege: USER-MODE\n");
    } else {
        print("  Privilege: KERNEL-MODE\n");
    }

    if (error_code & (1 << 3)) {
        print("  Error: Reserved bits set in page directory entries!\n");
    }

    // Намертво вешаем процессор
    asm volatile("cli; hlt");
}

__attribute__((interrupt)) void
invalide_opcode_handler(struct interrupt_frame *frame) {
// Выключаем прерывания, чтобы никто не помешал нам напечатать краш-дамп
    asm volatile("cli"); 

    print("\n!!! KERNEL PANIC: INVALID OPCODE (#UD) !!!\n");
    
    // frame->rip содержит точный адрес инструкции, на которой процессор споткнулся!
    print("  Failed at Instruction Pointer (RIP): 0x");
    printf("%h", frame->rip); 
    print("\n");

    // Выводим указатель стека, чтобы проверить на срыв
    print("  Stack Pointer (RSP):               0x");
    printf("%h", frame->rsp);
    print("\n");

    // Намертво останавливаем процессор
    while(1) {
        asm volatile("hlt");
    }
}
__attribute__((interrupt)) void
double_fault_handler(struct interrupt_frame *frame) {
  DoubleFaultError();
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
  if (PitTicks % 1000 == 0) {
    Timepit.PitTimerSecondsUp++;
  }
  CursorBlinkTicks++;
  if (CursorBlinkTicks >= CURSOR_BLINK_RATE) {
    CursorBlinkTicks = 0;
    CursorVisible = !CursorVisible; // Инвертируем состояние (1->0, 0->1)
    DrawConsoleCursor();            // Перерисовываем курсор
  }
  Timepit.PitTimerMiliSecondsUp++;
  outb(PIC1_COMMAND, PIC_EOI);
}

// Инициализация IDT – назначаем обработчики
void pit_init(int hz) {
  int divisor = 1193180 / hz; // Рассчитываем делитель
                              // выбор канала (нулевой канал)
  outb(PIT_CMD, 0x36);

  // отправка делителей сначала младший потом старший
  uint8_t low = (uint8_t)(divisor & 0xFF);
  uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

  outb(PIT_CH0, low);
  outb(PIT_CH0, high);
}
void idt_init() {

  for (int i = 0; i < 256; i++) {
    set_idt_gate(i, (void *)empty_handler); // нулевой указатель - может быть
                                            // заменён на заглушку
  }
  pic_remap();

  pit_init(1000);
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
