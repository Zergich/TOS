#include <System/OSInit.h>
#include <System/interrupts.h>
#include <VGA/shell.h>
#include <types.h>

#include <System/Array.h>
#include <System/time.h>
#include <VGA/print.h>
#include <string.h>
extern TimePit Timepit;
extern StringStruct string;

void kernel_main() {
  WelcomeMessage();
  idt_init();
  // После полной настройки прерываний включаем их
  asm volatile("sti");

  pit_init(1000);

  char str[] = "12345";
  int size = string.Strlen(str); // Убедитесь, что size правильный!
  IndexDeleteC(str, &size, 4);
  print(str);
  // int max_size = 10;
  // char arr[10];
  // int size = 0; // текущий размер массива
  //
  // // Вставляем элементы
  //
  // IndexInsertC(arr, &size, max_size, 1, 'e');
  // IndexInsertC(arr, &size, max_size, 2, 'd');
  // IndexInsertC(arr, &size, max_size, 3, 'e');
  // IndexInsertC(arr, &size, max_size, 0, '2');
  //
  // print(arr);
  // print("\n");
  // IndexDeleteC(arr, &size, 3);
  // print(arr);

  static string15 pede;
  while (true) {
    Shell();
    // Бесконечный цикл ядра
  }
}
