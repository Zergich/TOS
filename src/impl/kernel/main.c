#include <System/OSInit.h>
#include <System/interrupts.h>
#include <VGA/shell.h>
#include <types.h>

#include <System/Array.h>
#include <System/time.h>
#include <VGA/console.h>
#include <VGA/print.h>
#include <string.h>
extern TimePit Timepit;
extern StringStruct string;

void kernel_main() {
  WelcomeMessage();
  idt_init();
  // После полной настройки прерываний включаем их
  asm volatile("sti");

  struct Char pede1[100];
  print("pede");
  ConsoleBufferReadString(20, 40, 80, 1, pede1);
  struct Char *buffer = (struct Char *)0xb8000; // VGA память
  for (int i = 0; i < 20; i++) {
    buffer[i + 80 * 20] = pede1[i];
    PrintChar(pede1[i].character);
  }
  pit_init(1000); // прерывается 1000 раз в секунду

  static string15 pede;
  while (true) {
    Shell();
    // Бесконечный цикл ядра
  }
}
