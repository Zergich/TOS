#pragma once
#include <ConsoleIO/print.h>
#include <types.h>

// 1 - курсор видим, 0 - скрыт
extern u8 CursorVisible;

// Счетчик тиков таймера для управления скоростью моргания
extern u32 CursorBlinkTicks;

// Скорость моргания
extern u16 CURSOR_BLINK_RATE;

extern u32 *ActiveInputBuffer;
typedef struct {
  int (*ReadLine)(u32 *string);
  u32 (*ReadKey)(void);
} ConsoleInput;

extern u16 ShellStartRow; //  хрень нужна для обновления позиции при перемотке
                          //  для стрелок
int ConsoleRead(u32 *string);
u32 ReadKey();
void DrawConsoleCursor();
void ConsoleBufferReadString(u8 Start, u8 End, u8 MaxColumn, u8 Line);
