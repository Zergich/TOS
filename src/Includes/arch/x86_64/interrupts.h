#pragma once
#include <stdint.h>

void set_idt_gate(int n, void (*handler)(void));

void idt_init();
void load_idt();

/*
 * IDT - таблица прерываний
 *
 * PIC - Программируемый контроллер прерываний
 *
 */

// Порты PIT
#define PIT_CMD 0x43 // Командный порт
#define PIT_CH0 0x40 // Канал 0 (таймер)

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
// void outb(unsigned short port, unsigned char val);
// char inb(unsigned short port);
