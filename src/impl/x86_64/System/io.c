#include <System/io.h>

#pragma GCC target("general-regs-only") // запрет на поддержку SSE/AVX
                                        // выключение регистров XMM/YMM
uint8_t inb(uint16_t port) {
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

// Функция записи в порт
void outb(uint16_t port, uint8_t val) {
  asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}
