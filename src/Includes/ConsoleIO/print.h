#pragma once

#include <stddef.h>
#include <stdint.h>
#include <types.h>

enum {
  CONSOLE_COLOR_BLACK = 0x000000,
  CONSOLE_COLOR_BLUE = 0x0000AA,
  CONSOLE_COLOR_GREEN = 0x00AA00,
  CONSOLE_COLOR_CYAN = 0x00AAAA,
  CONSOLE_COLOR_RED = 0xAA0000,
  CONSOLE_COLOR_MAGENTA = 0xAA00AA,
  CONSOLE_COLOR_BROWN = 0xAA5500,
  CONSOLE_COLOR_LIGHT_GRAY = 0xAAAAAA,
  CONSOLE_COLOR_DARK_GRAY = 0x555555,
  CONSOLE_COLOR_LIGHT_BLUE = 0x5555FF,
  CONSOLE_COLOR_LIGHT_GREEN = 0x55FF55,
  CONSOLE_COLOR_LIGHT_CYAN = 0x55FFFF,
  CONSOLE_COLOR_LIGHT_RED = 0xFF5555,
  CONSOLE_COLOR_PINK = 0xFF55FF,
  CONSOLE_COLOR_YELLOW = 0xFFFF55,
  CONSOLE_COLOR_WHITE = 0xFFFFFF,
};

#define print(x)                                                               \
  _Generic((x),                                                                \
      int: print_int,                                                          \
      u64: print_unsigned,                                                     \
      char: print_char,                                                        \
      char *: print_str)(x)

void ConsoleClear();
// Вывод
void PrintChar(char character);
// для удобства и тчоб каждый раз не писать printf
void print_int(int value);
void print_unsigned(u64 value);
void print_char(char value);
void print_str(char *string); // с маленькой буквы для удобствa

void printf(char *string, ...);
void PrintDEC(uint64_t value);
void PrintHex64(uint64_t value);
void Print64Bin(uint64_t value);
void PrintINT(int value);
void PrintChar(char character);
void ConsoleScroll();

void PutChar(u64 x, u64 y, char character);

// Цвет
void ConsoleColor(u32 foreground, u32 background);
void ConsoleResetColor();
void ConsoleForeground(u32 foreground);
void ConsoleBackground(u32 background);

// Курсор
void CursorSetLine(u16 row);
void CursorSetColumn(u16 column);
void ConsoleSetCarretPos(u16 column, u16 row);
uint8_t CursorColumn();
uint8_t CursorLine();

// вывод ошибок
void PrintError(char *string);
