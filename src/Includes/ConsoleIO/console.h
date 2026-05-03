#pragma once
#include <ConsoleIO/print.h>
#include <types.h>

// 1 - курсор видим, 0 - скрыт
extern u8 CursorVisible;

// Счетчик тиков таймера для управления скоростью моргания
extern u32 CursorBlinkTicks;

// Скорость моргания
extern u16 CURSOR_BLINK_RATE;

extern char *ActiveInputBuffer;
typedef struct {
  int (*ReadLine)(char *string);
  char (*ReadKey)(void);
} ConsoleInput;

int ConsoleRead(char *string);
char ReadKey();
void DrawConsoleCursor();
void ConsoleBufferReadString(u8 Start, u8 End, u8 MaxColumn, u8 Line);
