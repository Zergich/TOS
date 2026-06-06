#include "types.h"
#include <ConsoleIO/console.h>
#include <ConsoleIO/font.h>
#include <ConsoleIO/graphics.h>
#include <ConsoleIO/print.h>
#include <System/sysinfo.h>
#include <VGA/vgacursor.h>
#include <libs/MemoryUtils.h>
#include <stdarg.h>
#include <stdint.h>

uint16_t CursorPosCol = 7;
uint16_t CursorPosRow = 3;
extern Pixeling PixelGrapchics;

// Цвета
u32 Foreground = CONSOLE_COLOR_WHITE;
u32 Background = CONSOLE_COLOR_BLACK;
u32 SaveColorFG = 0;
u32 SaveColorBG = 0;
//  сохранить цвет при его изменгением в функции ошибки

void ClearRow(size_t row) {

  for (size_t col = 0; col < NUM_COLUMS; col++) {
    DrawChar(col, row, 0, Background, Background);
  }
  CursorPosRow = 0;
}
void ConsoleClear() {
  for (size_t i = 0; i < NUM_ROWS; i++) {
    ClearRow(i);
  }
}

void PutChar(u64 x, u64 y, char character) {

  PixelGrapchics.DrawChar(x, y, character, Foreground, Background);
}

void ConsoleScroll() {
  u64 pixels_per_line = BufferPitchW;

  // Приводим указатель к u32* для правильной математики указателей
  u32 *buffer = (u32 *)PixelGrapchics.ptr;

  // Сдвигаем все пиксели, кроме последней текстовой строки, вверх на
  // FONT_HEIGHT пикселей
  u64 offset_to_src = pixels_per_line * FONT_HEIGHT;
  u64 pixels_to_move = pixels_per_line * (HeightDisplay - FONT_HEIGHT);

  memmove(buffer, buffer + offset_to_src, pixels_to_move * sizeof(u32));

  // Очищаем последнюю текстовую строку фоновым цветом
  u32 bg_color = Background;
  u64 start_of_last_line = (HeightDisplay - FONT_HEIGHT) * pixels_per_line;

  for (u64 i = 0; i < pixels_per_line * FONT_HEIGHT; i++) {
    buffer[start_of_last_line + i] = bg_color;
  }
  // обновление значения логики переменных отвечающих за стрелки
  if (ShellStartRow > 0) {
    ShellStartRow--;
  }
}

void PrintChar(char character) {
  // 1. Обработка переноса строки (Enter)

  if (CursorPosCol == NUM_COLUMS && CursorPosRow == NUM_ROWS)
    ConsoleScroll();
  // потому что если я что то пишу в конце всех строк
  // и потом пытается написать куда то в низ и ядро крашится я блять даже такой
  // ошибки процессора не хнаю это не ошибка страниц хз
  if (character == '\n') {
    CursorPosCol = 0;
    CursorPosRow++;
  }
  // 2. Обработка обычного символа
  else {
    // пиксельные координаты для отрисовки

    PixelGrapchics.DrawChar(CursorPosCol, CursorPosRow, character, Foreground,
                            Background);

    CursorPosCol++;
  }

  // 3. Проверка на достижение правого края экрана (автоматический перенос)
  if (CursorPosCol >= NUM_COLUMS) {
    CursorPosCol = 0;
    CursorPosRow++;
  }

  // 4. Проверка на достижение нижнего края экрана
  if (CursorPosRow >= NUM_ROWS) {
    ConsoleScroll();             // Двигаем экран вверх
    CursorPosRow = NUM_ROWS - 1; // Оставляем курсор на последней строке
  }

  // 5. Обновляем позицию (например, для аппаратного курсора, если он есть)
  CursorPos(CursorPosCol, CursorPosRow);
}
void print_int(int value) { printf("%i", value); }
void print_unsigned(u64 value) { printf("%u", value); }
void print_char(char value) { PrintChar(value); }
void print_str(char *string) { // это для базового ввода
  for (size_t i = 0; 1; i++) {
    char character = (uint8_t)string[i];
    if (character == '\0')
      return;

    PrintChar(character);
  }
}
void printf(char *string, ...) { // а это уже тяжелая артилерия
  va_list args;
  va_start(args, string);

  for (size_t i = 0; string[i] != '\0'; i++) {
    if (string[i] == '%') {
      i++; // следующий символ после '%'

      switch (string[i]) {
      case 'u': {
        int val = va_arg(args, unsigned); // Читаем int из аргументов
        if (val < 0) {
          PrintError("Get received a number less than zero in unsigned!");
          return;
        }
        PrintDEC((uint64_t)val); // Выводим через printDEC
        break;
      case 's':
        char *str = va_arg(args, char *);
        print(str);
        break;
      case 'i':
        int num = va_arg(args, int);
        PrintINT(num);
        break;
      case 'h':
        uint64_t hex = va_arg(args, uint64_t);
        PrintHex64(hex);
        break;
      case 'c':
        char character = va_arg(
            args, int); // char преобразуется в int при переводе с параметрами
                        // ... так char нежелательно использлвать на прямуюх
        PrintChar(character);
        break;
      case 'F': // так же если убрать break и добавить следом парметр ансигнед
                // то можно получить цвета которых нет в изначальной палитре
        // или просто в параметр цвета передать число какое нибудь но есть
        // вариант что оно модет сработать и на цвет заднего фона и на оборот
        uint64_t foreground = va_arg(args, uint64_t);
        ConsoleForeground(foreground);
        break;
      case 'B':
        uint64_t background = va_arg(args, uint64_t);
        ConsoleBackground(background);
        break;
      }
      // Можно добавить другие спецификаторы, например 's', 'c' и т.д.
      default:
        PrintChar('%');
        PrintChar(string[i]);
        break;
      }
    } else {
      PrintChar(string[i]); // Просто печатаем символ
    }
  }

  va_end(args);
}

void ConsoleColor(u32 foreground, u32 background) {
  Foreground = foreground;
  Background = background;
}
void ConsoleSetCarretPos(u16 column, u16 row) {
  CursorPosCol = column;
  CursorPosRow = row;
  CursorPos(column, row);
  CursorPosCol = column;
  CursorPosRow = row;
}
void CursorSetLine(u16 row) { CursorPosRow = row; }
void CursorSetColumn(u16 column) { CursorPosCol = column; }

uint8_t CursorLine() { return CursorPosRow; }
uint8_t CursorColumn() { return CursorPosCol; }

void PrintDEC(uint64_t value) {
  if (value == 0) {
    PrintChar('0');
    return;
  }

  char buffer[20];
  int i = 0;

  while (value > 0) {
    buffer[i++] = (value % 10) + '0';
    value /= 10;
  }

  while (i-- > 0) {
    PrintChar(buffer[i]);
  }
}

void PrintHex64(uint64_t value) { // мб ужалить потому что не используется
  if (value == 0) {
    PrintChar('0');
    return;
  }

  char buffer[16];
  int i = 0;

  while (value > 0) {
    uint8_t digit = value & 0xF;

    if (digit < 10) {
      buffer[i++] = digit + '0';
    } else {
      buffer[i++] = digit - 10 + 'A';
    }

    value >>= 4;
  }

  while (i-- > 0) {
    PrintChar(buffer[i]);
  }
}

void Print64Bin(uint64_t value) { // мб ужалить потому что не используется
  char buffer[64];

  for (size_t i = 0; i < 64; i++) {
    buffer[i] = (value & 1) + '0';
    value >>= 1;
  }

  for (size_t i = 64; i > 0; i--) {
    PrintChar(buffer[i - 1]);
  }
}

void PrintError(char *string) {
  SaveColorFG = Foreground;
  SaveColorBG = Background;
  ConsoleColor(CONSOLE_COLOR_RED, CONSOLE_COLOR_BLACK);
  print("ERROR:\n");
  print(string);
  Background = SaveColorBG;
  Foreground = SaveColorFG;
}

void PrintINT(int value) {
  if (value == 0) {
    PrintChar('0');
    return;
  }
  if (value < 0) {
    PrintChar('-');
    // Преобразуем в положительное для удобства
    value = -value;
  }
  char buffer[20];
  int i = 0;

  while (value > 0) {
    buffer[i++] = (value % 10) + '0';
    value /= 10;
  }

  while (i-- > 0) {
    PrintChar(buffer[i]);
  }
}
void ConsoleResetColor() {
  Foreground = CONSOLE_COLOR_WHITE; // синхронизация цвета
  Background = CONSOLE_COLOR_BLACK;
}

// тут проблема в том что изначально мо человечески это сделать не получаеься
// потому что компилятор хочет чтоб переменные которые задействуюся в цвете
// (передний|задний) были константнымих
void ConsoleForeground(u32 foreground) {
  Foreground = foreground; // сохранение цвета для его синхронизации
}

void ConsoleBackground(u32 background) {
  Background = background; // сохранение цвета для его синхронизации
}
