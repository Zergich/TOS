#include <OSInit.h>
#include <bool.h>
#include <interrupts.h>
#include <print.h>
void kernel_main() {
  ConsoleClear();
  WelcomeMessage();

  idt_init();
  // test_interrupt();
  int a = 2 / 0;

  // Для проверки — вызов деления на ноль
  // Это вызовет обработчик
}
