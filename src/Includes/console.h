#pragma once
#include <types.h>

typedef struct {
  int (*ReadLine)(char *string);
  char (*ReadKey)(void);
} ConsoleInput;

int ConsoleRead(char *string);
char ReadKey();
