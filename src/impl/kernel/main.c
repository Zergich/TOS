#include "System/sysinfo.h"
#include <System/OSInit.h>
#include <System/interrupts.h>
#include <VGA/shell.h>
#include <types.h>

#include <System/time.h>
#include <string.h>
extern TimePit Timepit;
extern StringStruct string;

void kernel_main(u32 mb2_magic, u64 mb2_info_addr) {
  WelcomeMessage();
  idt_init();

  // magic байт для проверки интерфейса загрузки и ссылка на структуры для
  // чтения
  Mb2InfoAddr = mb2_info_addr;
  Mb2Magic = mb2_magic;

  // После полной настройки прерываний включаем их
  asm volatile("sti");

  pit_init(1000); // прерывается 1000 раз в секунду

  static string15 pede;
  while (true) {
    Shell();
    // Бесконечный цикл ядра
  }
}
