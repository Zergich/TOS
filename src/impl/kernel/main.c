#include <OSInit.h>
#include <bool.h>
#include <interrupts.h>
#include <print.h>
void kernel_main() {
  ConsoleClear();
  WelcomeMessage();

  idt_init();
  // После полной настройки прерываний включаем их
  asm volatile("sti");

  while (1) {
    // Бесконечный цикл ядра
  }
}
