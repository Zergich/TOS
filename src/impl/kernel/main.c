#include <OSInit.h>
#include <console.h>
#include <interrupts.h>
#include <print.h>
#include <stdint.h>
#include <string.h>
#include <types.h>
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

  char pede[100];
  while (true) {
    ConsoleRead(pede);
    if (strcmp(pede, "pede") == 0)
      print("pede123");
    printf("Shell> ");
    // Бесконечный цикл ядра
  }
}
