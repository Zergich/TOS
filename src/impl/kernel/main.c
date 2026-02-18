#include <OSInit.h>
#include <bool.h>
#include <interrupts.h>
#include <print.h>
#include <stdint.h>
#include <vgacursor.h>

extern uint16_t CursorPosCol;
extern uint16_t CursorPosRow;

void kernel_main() {
  ConsoleClear();
  WelcomeMessage();

  idt_init();
  CursorPosCol = 7;
  CursorPosRow = 3;
  CursorPos(CursorPosCol, CursorPosRow);
  // После полной настройки прерываний включаем их
  asm volatile("sti");

  while (1) {
    // Бесконечный цикл ядра
  }
}
