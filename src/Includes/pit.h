#ifndef PIT_H
#define PIT_H

#include "interrupts.h"

/* Инициализируйте ПИТ с заданной частотой (Гц) */
void pit_init(unsigned int frequency);

/* Обработчик прерываний по таймеру (IRQ0, вектор 32) */
void interrupt_handler_timer(void);

#endif
