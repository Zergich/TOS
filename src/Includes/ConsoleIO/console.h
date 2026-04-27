#pragma once
#include <ConsoleIO/print.h>
#include <types.h>

typedef struct {
  int (*ReadLine)(char *string);
  char (*ReadKey)(void);
} ConsoleInput;

int ConsoleRead(char *string);
char ReadKey();

void ConsoleBufferReadString(u8 Start, u8 End, u8 MaxColumn, u8 Line,
                             struct Char *ReadedBuffer);
