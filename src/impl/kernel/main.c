#include <print.h>

void kernel_main() {
  ConsoleClear();
  ConsoleColor(CONSOLE_COLOR_GREEN, CONSOLE_COLOR_BLACK);
  print("Timur ti gandon ara! \npede 123");
  ConsoleSetCursorPos(50, 10);
  printf("pede  %u", 10);
  PrintError("Alarmo Alarmo! Obnarushen Timur");
  printf("\n123 %s", "8133ujoauufdu\n");

  int test_1 = -343;
  char *str = "frau da";
  uint64_t uns = 1000;
  printf("Testing %i, %s, %u", test_1, str, uns);
}
