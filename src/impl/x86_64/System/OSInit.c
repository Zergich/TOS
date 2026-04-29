#include "types.h"
#include <ConsoleIO/print.h>
#include <System/OSInit.h>
#include <System/sysinfo.h>
#include <VGA/vgacursor.h>

// Инициализация констант

char *VersionOS = "ALPHA";
const u8 NUM_COLUMS = 80;
const u8 NUM_ROWS = 25;

u64 WidthDisplay = 0;
u64 HiegthDisplay = 0;

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
extern u16 CursorPosCol;
extern u16 CursorPosRow;

void ApperLine() {
  ConsoleColor(CONSOLE_COLOR_LIGHT_GREEN, CONSOLE_COLOR_BLACK);
  for (u8 i = 0; i < NUM_COLUMS; i++)
    print("-");
  print("\n");
  ConsoleResetColor();
  ConsoleForeground(CONSOLE_COLOR_CYAN);
  print("Shell> ");

  CursorPosCol = 7;
  CursorPosRow = 3;
  CursorPos(CursorPosCol, CursorPosRow);
}

// задумка пиздатая но пока не нашел ей действительно стоющего применения
void InitConstantGraphics(u64 Width, u64 Hiegth) {

  HiegthDisplay = Hiegth;
  WidthDisplay = Width;
}
