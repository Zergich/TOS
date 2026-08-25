#include <arch/x86_64/io.h>
#include <types.h>
#pragma GCC target("general-regs-only") // запрет на поддержку SSE/AVX
                                        // выключение регистров XMM/YMM
uint8_t inb(u16 port) {
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}
u16 inw(u16 port) {
  u16 ret;
  asm volatile("inw &1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

// Функция записи в порт
// для 8 бит
void outb(u16 port, u8 val) {
  asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}
// для 16 бит
void outw(u16 port, u16 val) {
  asm volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}
