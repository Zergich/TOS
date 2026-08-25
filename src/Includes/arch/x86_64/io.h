#pragma once
#include <stdint.h>
#include <types.h>
uint8_t inb(uint16_t port);
// Функция записи в порт
void outb(uint16_t port, uint8_t val);
void outw(u16 port, u16 val);
u16 inw(u16 port);
