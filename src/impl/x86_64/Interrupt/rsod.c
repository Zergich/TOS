#include <ConsoleIO/graphics.h>
#include <ConsoleIO/print.h>
#include <System/rsod.h>
#include <System/sysinfo.h>

#pragma GCC target("general-regs-only") // запрет на поддержку SSE/AVX
                                        // выключение регистров XMM/YMM
u64 CenterColPading;
u64 CenterRowPading;
void PrintBaner() // из за того что если строку записать как нормальный явный
                  // массив и потом по индексам брать то выводит хуйню
                  // раньше тупо было 5 переменных
{
  CenterColPading =
      (NUM_COLUMS - 30) / 2; // смещение для того чтоб нарисовать выше центра
  CenterRowPading = (NUM_ROWS - 20) / 2;

  char arr[5][10] = {"   (\n", "O (\n", "  (\n", "O (\n", "   (\n"};
  for (uint8_t i = 0; i < 5; i++) {
    ConsoleSetCarretPos(CenterColPading, CenterRowPading + i);
    print(arr[i]);
  }
}
void ErrorFone() {
  ConsoleClear();
  ConsoleColor(CONSOLE_COLOR_BLACK, CONSOLE_COLOR_RED);

  // раньше было так и было рвано и долго
  // ConsoleSetCarretPos(0, 0);
  // for (uint8_t i = 0; i < NUM_ROWS; i++) {
  //   for (uint8_t j = 0; j < NUM_COLUMS; j++) {
  //     print(" ");
  //   }
  //   print("\n");
  // }
  DrawPixel(0, HeightDisplay * WidthDisplay, CONSOLE_COLOR_RED);
  PrintBaner();
  ConsoleSetCarretPos(CenterColPading + 4, CenterRowPading + 2);
}
void DivideZero() {
  ErrorFone();
  print("INT 0. Processor error. Division by zero."); // Ошибка процессора.
  // // Деление на ноль.
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
