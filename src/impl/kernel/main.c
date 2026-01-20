#include <print.h>

void kernel_main() {
  ConsoleClear();
  ConsoleColor(CONSOLE_COLOR_GREEN, CONSOLE_COLOR_BLACK);
  print("Timur ti gandon ara! \npede 123");
  ConsoleSetCursorPos(9, 14);
  print("ara");
  ConsoleSetCursorPos(50, 10);
  print("ara \ndouble\n");
  print_uint64_dec(CursorColumn());
}
