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

  static string pede;
  while (true) {
    Shell();
    Timepit.UpTime();
    // Бесконечный цикл ядра
  }
}
