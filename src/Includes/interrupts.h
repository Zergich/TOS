
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

/*
 * IDT - таблица прерываний
 *
 * PIC - Программируемый контроллер прерываний
 *
 */

// IDT струутура
struct idt_entry {
  unsigned short base_lo;
  unsigned short sel;
  unsigned char always0;
  unsigned char flags;
  unsigned short base_hi;
} __attribute__((packed));

// IDT указатели
struct idt_ptr {
  unsigned short limit;
  unsigned int base;
} __attribute__((packed));

// Флаги
#define IDT_FLAG_PRESENT 0x80
#define IDT_FLAG_RING0 0x00
#define IDT_FLAG_INTGATE 0x0E
#define IDT_FLAG_FLAGS (IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INTGATE)

// PIC Порты
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20

// Декларированные функции
void init_idt(void);
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel,
                  unsigned char flags);
void pic_init(void);
void pic_ack(unsigned int irq);
// extern void outb(unsigned short port, unsigned char val);
// extern unsigned char inb(unsigned short port);
void interrupt_handler_keyboard(void);
void exception_handler(unsigned int interrupt_no, unsigned int error_code);

// Прерывания
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
extern void isr33(void);

#endif
