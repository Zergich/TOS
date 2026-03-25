#include <System/OSInit.h>
#include <System/sysinfo.h>
#include <VGA/print.h>
#include <VGA/vgacursor.h>

char *VersionOS = "0.0.1";

void WelcomeMessage() {
  ConsoleClear();
  ConsoleSetCarretPos(NUM_COLUMS / 3, 0); // центрирование
  ConsoleColor(CONSOLE_COLOR_GREEN, CONSOLE_COLOR_BLACK);
  print("Welcome to TOS!\n");
  ConsoleSetCarretPos(NUM_COLUMS / 3, 1);
  printf("OS Version: %s\n", VersionOS);
  ApperLine();
}

// надо потому что иначе начальное значние убивается и после первого символа
// ввода с клавы курсор улетает на хуйх
extern uint16_t CursorPosCol;
extern uint16_t CursorPosRow;

void ApperLine() {
  ConsoleColor(CONSOLE_COLOR_LIGHT_GREEN, CONSOLE_COLOR_BLACK);
  for (uint8_t i = 0; i < NUM_COLUMS; i++)
    print("-");
  print("\n");
  ConsoleResetColor();
  ConsoleForeground(CONSOLE_COLOR_CYAN);
  print("Shell> ");

  CursorPosCol = 7;
  CursorPosRow = 3;
  CursorPos(CursorPosCol, CursorPosRow);
}
