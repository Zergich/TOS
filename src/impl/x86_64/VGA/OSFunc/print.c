#include <print.h>
#include <stdarg.h>
#include <stdint.h>
#include <sysinfo.h>
#include <vgacursor.h>

extern uint16_t CursorPosCol;
extern uint16_t CursorPosRow;

// const static size_t NUM_COLS = 80;
// const static size_t NUM_ROWS = 25;
const size_t NUM_COLUMS = 80;
const size_t NUM_ROWS = 25;

struct Char {
  uint8_t character;
  uint8_t color;
};

struct Char *buffer = (struct Char *)0xb8000; // VGA память
size_t VGA_Column = 0;
size_t VGA_Line = 0;

// Цвета
uint8_t Foreground = CONSOLE_COLOR_WHITE;
uint8_t Background = CONSOLE_COLOR_BLACK;
uint8_t color = CONSOLE_COLOR_WHITE | CONSOLE_COLOR_BLACK << 4;
uint8_t SaveColor = 0;
//  сохранить цвет при его изменгением в функции ошибки

void ClearRow(size_t row) {
  struct Char EmptySymbol = (struct Char){
    character : ' ',
    color : color,
  };
  for (size_t col = 0; col < NUM_COLUMS; col++) {
    buffer[col + NUM_COLUMS * row] = EmptySymbol;
  }
}
void ConsoleClear() {
  for (size_t i = 0; i < NUM_ROWS; i++) {
    ClearRow(i);
  }
}

void ConsoleRePrintDown() { // переписывает буффер консоли (строка дошла до
                            // конца). пролистывает вниз
  VGA_Column = 0;
  if (VGA_Line < NUM_ROWS - 1) {
    VGA_Line++;
    return;
  }
  // здесь + 1 один потому что без него функция копирует последнюю
  // строку и втасляет ее заново. то есть на новой строке у тебя
  // было то что на пердыдущей
  for (size_t row = 1; row < NUM_ROWS + 1; row++) {
    for (size_t col = 0; col < NUM_COLUMS; col++) {
      struct Char character = buffer[col + NUM_COLUMS * row];
      buffer[col + NUM_COLUMS * (row - 1)] = character;
    }
  }
  ClearRow(NUM_COLUMS - 1);
  CursorPosCol = 0;
  CursorPosRow = NUM_ROWS - 2; // курсор просто пропадает без этой штуки
}

void PrintChar(char character) {
  if (character == '\n') {
    ConsoleRePrintDown();
    CursorPosCol = 0;
    // тут дело в суфиксах и постфиксах потому что если Х++ то сначала
    // передается значение в функции, функция отрабатывает с этим значением а
    // потом только увеличивает его а если ++Х то сначала увеличивает а потом
    // работает
    CursorPos(CursorPosCol, ++CursorPosRow);
    return;
  }
  if (VGA_Column > NUM_COLUMS) {
    ConsoleRePrintDown();
    CursorPosCol =
        1; // из за какойто херни когда автоматически переходит на новую строку
           // при заполнении пердыдущей строки печатается первый символ в
           // позиции 0 а следующей символ встает на эту же позицию таким
           // образом скпивая 1 символ по этому переменная равна 1 а не 0 и в
           // конце тела колона инкрементируется
    CursorPosRow++;
    CursorPos(CursorPosCol, CursorPosRow);
    VGA_Column++;
  }
  buffer[VGA_Column + NUM_COLUMS * VGA_Line] = (struct Char){
    character : (uint8_t)character,
    color : color,
  };
  VGA_Column++;

  CursorPos(++CursorPosCol, CursorPosRow);
}

void print(char *string) { // это для базового ввода
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

void ConsoleColor(uint8_t foreground, uint8_t background) {
  color = foreground + (background << 4);
}
void ConsoleSetCarretPos(uint8_t column, uint8_t row) {
  VGA_Column = column;
  VGA_Line = row;
}
void CursorSetLinePos(uint8_t row) { VGA_Line = row; }
void CursorSetColumn(uint8_t column) { VGA_Column = column; }

uint8_t CursorLine() { return VGA_Line; }
uint8_t CursorColumn() { return VGA_Column; }

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
  SaveColor = color;
  ConsoleColor(CONSOLE_COLOR_RED, CONSOLE_COLOR_BLACK);
  print("ERROR:\n");
  print(string);
  color = SaveColor;
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
  color = CONSOLE_COLOR_WHITE | CONSOLE_COLOR_BLACK << 4;
}

// тут проблема в том что изначально мо человечески это сделать не получаеься
// потому что компилятор хочет чтоб переменные которые задействуюся в цвете
// (передний|задний) были константнымих
void ConsoleForeground(uint8_t foreground) {
  Foreground = foreground; // сохранение цвета для его синхронизации
  color = foreground | Background << 4;
}

void ConsoleBackground(uint8_t background) {
  Background = background; // сохранение цвета для его синхронизации
  color = Foreground | background << 4;
}
