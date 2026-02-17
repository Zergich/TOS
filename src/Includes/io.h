#pragma once
#include <stdint.h>

uint8_t inb(uint16_t port);
// Функция записи в порт
void outb(uint16_t port, uint8_t val);
