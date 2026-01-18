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
void PrintChar(char character);
void print(char *string); // с маленькой буквы для удобства
void ConsoleColor(uint8_t foreground, uint8_t background);
