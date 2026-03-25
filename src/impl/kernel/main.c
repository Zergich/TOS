#include <System/OSInit.h>
#include <System/interrupts.h>
#include <VGA/shell.h>
#include <types.h>

#include <System/time.h>

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
