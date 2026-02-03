#pragma once

#include <stddef.h>
#include <stdint.h>

enum {
  CONSOLE_COLOR_BLACK = 0,
  CONSOLE_COLOR_BLUE = 1,
  CONSOLE_COLOR_GREEN = 2,
  CONSOLE_COLOR_CYAN = 3,
  CONSOLE_COLOR_RED = 4,
  CONSOLE_COLOR_MAGENTA = 5,
  CONSOLE_COLOR_BROWN = 6,
  CONSOLE_COLOR_LIGHT_GRAY = 7,
  CONSOLE_COLOR_DARK_GRAY = 8,
  CONSOLE_COLOR_LIGHT_BLUE = 9,
  CONSOLE_COLOR_LIGHT_GREEN = 10,
  CONSOLE_COLOR_LIGHT_CYAN = 11,
  CONSOLE_COLOR_LIGHT_RED = 12,
  CONSOLE_COLOR_PINK = 13,
  CONSOLE_COLOR_YELLOW = 14,
  CONSOLE_COLOR_WHITE = 15,
};

void ConsoleClear();
// Вывод в VGA
void PrintChar(char character);
void print(char *string); // с маленькой буквы для удобствa
void printf(char *string, ...);
void printDEC(uint64_t value);
void PrintHex64(uint64_t value);
void Print64Bin(uint64_t value);
void PrintINT(int value);

// Цвет
void ConsoleColor(uint8_t foreground, uint8_t background);
void ConsoleResetColor();
void ConsoleForeground(uint8_t foreground);
void ConsoleBackground(uint8_t background);

// Курсор
void ConsoleSetCursorPos(uint8_t column, uint8_t row);
uint8_t CursorColumn();
uint8_t CursorLine();

// вывод ошибок
void PrintError(char *string);
