
#include "interrupts.h"
#include <stdint.h>

static struct idt_entry idt[256];
static struct idt_ptr idtp;

extern void idt_load(struct idt_ptr *ptr);
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);
extern void isr32(void);
extern void isr33(void);

void init_idt(void) {
  idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
  idtp.base = (uintptr_t)&idt;

  // Очистить IDT
  for (int i = 0; i < 256; i++) {
    idt_set_gate(i, 0, 0x10, 0); // Отсутствует по умолчанию
  }

  // Установить обработчики исключений
  idt_set_gate(0, (unsigned long)isr0, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(1, (unsigned long)isr1, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(2, (unsigned long)isr2, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(3, (unsigned long)isr3, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(4, (unsigned long)isr4, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(5, (unsigned long)isr5, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(6, (unsigned long)isr6, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(7, (unsigned long)isr7, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(8, (unsigned long)isr8, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(9, (unsigned long)isr9, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(10, (unsigned long)isr10, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(11, (unsigned long)isr11, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(12, (unsigned long)isr12, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(13, (unsigned long)isr13, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(14, (unsigned long)isr14, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(15, (unsigned long)isr15, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(16, (unsigned long)isr16, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(17, (unsigned long)isr17, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(18, (unsigned long)isr18, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(19, (unsigned long)isr19, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(20, (unsigned long)isr20, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(21, (unsigned long)isr21, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(22, (unsigned long)isr22, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(23, (unsigned long)isr23, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(24, (unsigned long)isr24, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(25, (unsigned long)isr25, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(26, (unsigned long)isr26, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(27, (unsigned long)isr27, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(28, (unsigned long)isr28, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(29, (unsigned long)isr29, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(30, (unsigned long)isr30, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(31, (unsigned long)isr31, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(32, (unsigned long)isr32, 0x10, IDT_FLAG_FLAGS);
  idt_set_gate(33, (unsigned long)isr33, 0x10, IDT_FLAG_FLAGS);

  idt_load(&idtp);
}

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel,
                  unsigned char flags) {
  idt[num].base_lo = (base & 0xFFFF);
  idt[num].base_hi = (base >> 16) & 0xFFFF;
  idt[num].sel = sel;
  idt[num].always0 = 0;
  idt[num].flags = flags;
}
