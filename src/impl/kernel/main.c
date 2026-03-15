#include <OSInit.h>
#include <interrupts.h>
#include <shell.h>
#include <types.h>

#include <print.h>
#include <time.h>
extern TimePit Timepit;
void kernel_main() {
  WelcomeMessage();

  idt_init();
  // После полной настройки прерываний включаем их
  asm volatile("sti");

  pit_init(1000);
  static string pede;
  while (true) {
    Shell();
    // Бесконечный цикл ядра
  }
}
