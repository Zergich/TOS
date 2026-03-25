#include <VGA/print.h>
#include <VGA/rsod.h>

#pragma GCC target("general-regs-only") // запрет на поддержку SSE/AVX
                                        // выключение регистров XMM/YMM

void PrintBaner() // из за того что если строку записать как нормальный явный
                  // массив и потом по индексам брать то выводит хуйню
// раньше тупо было 5 переменных
{
  char arr[5][10] = {"   (\n", "O (\n", "  (\n", "O (\n", "   (\n"};
  for (uint8_t i = 0; i < 5; i++) {
    ConsoleSetCarretPos(10, 5 + i);
    print(arr[i]);
  }
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
