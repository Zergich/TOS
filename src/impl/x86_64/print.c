#include <print.h>

const static size_t NUM_COLS = 80;
const static size_t NUM_ROWS = 25;

struct Char {
  uint8_t character;
  uint8_t color;
};

struct Char *buffer = (struct Char *)0xb8000;
size_t Column = 0;
size_t Line = 0;
uint8_t color = CONSOLE_COLOR_WHITE | CONSOLE_COLOR_BLACK << 4;

void ClearRow(size_t row) {
  struct Char EmptySymbol = (struct Char){
    character : ' ',
    color : color,
  };
  for (size_t col = 0; col < NUM_COLS; col++) {
    buffer[col + NUM_COLS * row] = EmptySymbol;
  }
}
void ConsoleClear() {
  for (size_t i = 0; i < NUM_ROWS; i++) {
    ClearRow(i);
  }
}

void ConsoleRePrintDown() { // переписывает буффер консоли (строка дошла до
                            // конца). пролистывает вниз
  Column = 0;
  if (Line < NUM_ROWS - 1) {
    Line++;
    return;
  }
  for (size_t row = 1; row < NUM_ROWS; row++) {
    for (size_t col = 0; col < NUM_COLS; col++) {
      struct Char character = buffer[col + NUM_COLS * row];
      buffer[col + NUM_COLS * (row - 1)] = character;
    }
  }
  ClearRow(NUM_COLS - 1);
}

void PrintChar(char character) {
  if (character == '\n') {
    ConsoleRePrintDown();
    return;
  }
  if (Column > NUM_COLS) {
    ConsoleRePrintDown();
  }
  buffer[Column + NUM_COLS * Line] = (struct Char){
    character : (uint8_t)character,
    color : color,
  };
  Column++;
}

void print(char *string) {
  for (size_t i = 0; 1; i++) {
    char character = (uint8_t)string[i];
    if (character == '\0')
      return;

    PrintChar(character);
  }
}

void ConsoleColor(uint8_t foreground, uint8_t background) {
  color = foreground + (background << 4);
}
void ConsoleSetCursorPos(uint8_t column, uint8_t row) {
  Column = column;
  Line = row;
}

uint8_t CursorLine() { return Line; }
uint8_t CursorColumn() { return Column; }

void print(uint64_t value) {
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

void PrintHex64(uint64_t value) {
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

void Print64Bin(uint64_t value) {
  char buffer[64];

  for (size_t i = 0; i < 64; i++) {
    buffer[i] = (value & 1) + '0';
    value >>= 1;
  }

  for (size_t i = 64; i > 0; i--) {
    PrintChar(buffer[i - 1]);
  }
}
