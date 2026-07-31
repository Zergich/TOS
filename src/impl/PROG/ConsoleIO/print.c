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

// старая функция для uft не годится но она по прежнему используется
void PutChar(u64 x, u64 y, u32 character) {

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

void PrintChar(u32 character) {
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
void print_float(float number) { print_double((double)number); }
void print_double(double number) {
  if (number < 0) {
    print_char('-');
    number = -number;
  }

  // Получаем целую часть простым приведением типов (оно отсекает хвост)
  uint64_t integer_part = (uint64_t)number;

  // Получаем дробную часть (например, из 125.53 вычитаем 125.0 -> получаем
  // 0.53)
  double fraction = number - (double)integer_part;

  // Выделяем нужное количество знаков после запятой (например, 1 знак для
  // памяти: 0.53 * 10 = 5.3 -> 5)
  uint64_t fractional_part =
      (uint64_t)(fraction * 10.0 + 0.5); // +0.5 для правильного округления

  // Если округление округлило дробь до 10 (например, было 125.96 -> стало .10),
  // корректируем целую часть.
  if (fractional_part >= 10) {
    integer_part += 1;
    fractional_part = 0;
  }

  // Выводим через твои базовые функции (чтобы не завязываться на логику самого
  // printf)
  print_unsigned(integer_part);
  print_char('.');
  print_unsigned(fractional_part);
}

void print_unsigned(u64 value) { printf("%u", value); }
void print_char(char value) { PrintChar(value); }
void print_str_u32_native(u32 *str) {
  for (int i = 0; str[i] != 0; i++) {
    PrintChar(str[i]); // Всё! Никаких поисков глифов перед вызовом.
  }
}
void print_str(char *StringData) {
  while (1) {
    u32 characterNumber = Uft8Decoder(&StringData);
    if (characterNumber == '\0')
      return;
    if (characterNumber == '\n') {
      PrintChar('\n');
      continue;
    }
    PrintChar(characterNumber);
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
      } // ИСПРАВЛЕНО: здесь не хватало закрывающей скобки для блока case 'u'
      case 'S':
        char *strC = va_arg(args, char *);

        print(strC);
        break;
      case 's':
        u32 *str = va_arg(args, u32 *);

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
                        // ... так char нежелательно использлвать на прямую
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
      // ИСПРАВЛЕНО: здесь стояла лишняя скобка '}', которая ломала switch
      case 'f':
        double NumberDouble = va_arg(args, double);
        print_double(NumberDouble);
        break;
      // Можно добавить другие спецификаторы, например 's', 'c' и т.д.
      default:
        PrintChar('%');
        PrintChar(string[i]);
        break;
      }
    } else {
      // --- ИНТЕГРАЦИЯ UTF-8 ---

      // Берем адрес текущего символа в памяти
      char *ptr = (char *)&string[i];
      char *old_ptr = ptr; // Запоминаем, где мы были

      // Твой декодер прочитает 1, 2, 3 или 4 байта и сам сдвинет ptr вперед
      u32 code = Uft8Decoder(&ptr);

      // Жестко перехватываем перенос строки, чтобы избежать глифа 'î' (индекс
      // 10)
      if (code == '\n') {
        // Замени на свою функцию перевода строки, если PrintChar это не умеет
        // Например: CursorPosRow += 16; CursorPosCol = LeftPadding;
        PrintChar('\n');
      } else if (code == '\r') {
        // Пропускаем возврат каретки
      } else {
        // Ищем индекс картинки в шрифте и рисуем
        PrintChar(code);
      }

      // МАГИЯ СДВИГА:
      // Допустим, была русская буква (2 байта). ptr сдвинулся на 2 шага.
      // Нам нужно прибавить к индексу 'i' эту разницу минус 1.
      // Почему минус 1? Потому что сам цикл for сделает i++ в конце итерации!
      i += (ptr - old_ptr) - 1;
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
  Foreground = CONSOLE_COLOR_CYAN; // синхронизация цвета
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

static inline u8 hex_char_to_int(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return 0;
}

u32 HexColor(char *hex_str) {
  if (hex_str == 0)
    return 0;

  u32 index = 0;

  if (hex_str[index] == '#') {
    index++;
  } else if (hex_str[index] == '0' &&
             (hex_str[index + 1] == 'x' || hex_str[index + 1] == 'X')) {
    index += 2;
  }

  u32 result_color = 0;

  while (hex_str[index] != '\0') {
    char current_char = hex_str[index];

    if ((current_char >= '0' && current_char <= '9') ||
        (current_char >= 'a' && current_char <= 'f') ||
        (current_char >= 'A' && current_char <= 'F')) {
      result_color = (result_color << 4) | hex_char_to_int(current_char);
      index++;
    } else {
      PrintError("Hex Error");
      break;
    }
  }

  if (index == 6 || (hex_str[0] == '#' && index == 7) ||
      (hex_str[1] == 'x' && index == 8)) {
    result_color |= 0xFF000000;
  }

  return result_color;
}
