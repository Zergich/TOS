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
  ConsoleSetCarretPos(10, 5);
  print(Baner);
  ConsoleSetCarretPos(10, 6);
  print(Baner2);
  ConsoleSetCarretPos(10, 7);
  print(Baner3);
  ConsoleSetCarretPos(10, 8);
  print(Baner4);
  ConsoleSetCarretPos(10, 9);
  print(Baner5);
  // какой позор а что поделать
}
void ErrorFone() {
  ConsoleClear();
  ConsoleColor(CONSOLE_COLOR_BLACK, CONSOLE_COLOR_RED);
  ConsoleSetCarretPos(0, 0);
  for (uint8_t i = 0; i < 25; i++) {
    for (uint8_t j = 0; j < 80; j++) {
      print(" ");
    }
    print("\n");
  }
  PrintBaner();
  ConsoleSetCarretPos(16, 7);
}

void DivideZero() {
  ErrorFone();
  print("INT 0. Processor error. Division by zero."); // Ошибка процессора.
                                                      // Деление на ноль.
}

void MappingError() {
  ErrorFone();
  print("INT 14. Mapping memory error.");
}
void OpcodeError() {
  ErrorFone();
  print("INT 6. Invalid Opcode");
}
void DoubleFaultError() {
  ErrorFone();
  print("INT 8. Double Fault");
}
void GeneralProtectionError() {
  ErrorFone();
  print("INT 13. Null pointer or writing in read-only memory, or violation of "
        "privileges");
}
