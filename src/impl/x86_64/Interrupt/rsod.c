#include <print.h>
#include <rsod.h>
#include <stdint.h>

void PrintBaner() // из за того что если строку записать как нормальный явный
                  // массив и потом по индексам брать то выводит хуйню
{
  char *Baner = "   (\n";
  char *Baner2 = "O (\n";
  char *Baner3 = "  (\n";
  char *Baner4 = "O (\n";
  char *Baner5 = "   (\n";
  ConsoleSetCursorPos(10, 5);
  print(Baner);
  ConsoleSetCursorPos(10, 6);
  print(Baner2);
  ConsoleSetCursorPos(10, 7);
  print(Baner3);
  ConsoleSetCursorPos(10, 8);
  print(Baner4);
  ConsoleSetCursorPos(10, 9);
  print(Baner5);
  // какой позор а что поделать
}

void DivideZero() {
  ConsoleClear();
  ConsoleColor(CONSOLE_COLOR_BLACK, CONSOLE_COLOR_RED);
  ConsoleSetCursorPos(0, 0);
  for (uint8_t i = 0; i < 25; i++) {
    for (uint8_t j = 0; j < 80; j++) {
      print(" ");
    }
    print("\n");
  }
  PrintBaner();
  ConsoleSetCursorPos(16, 7);
  print("Processor error. Division by zero."); // Ошибка процессора. Деление на
                                               // ноль.
  asm volatile("hlt");
}
