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
