#include <print.h>

const static size_t NUM_COLS = 80;
const static size_t NUM_ROWS = 25;

struct Char {
  uint8_t character;
  uint8_t color;
};

struct Char *buffer = (struct Char *)0xb8000;
size_t col = 0;
size_t row = 0;
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

void PrintNewLine() {
  col = 0;
  if (row < NUM_ROWS - 1) {
    row++;
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
    PrintNewLine();
    return;
  }
  if (col > NUM_COLS) {
    PrintNewLine();
  }
  buffer[col + NUM_ROWS * row] = (struct Char){
    character : (uint8_t)character,
    color : color,
  };
  col++;
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
