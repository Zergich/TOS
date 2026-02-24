#pragma once
#include <types.h>

typedef struct {
  char *(*ReadLine)(void);
  char (*ReadKey)(void);
} ConsoleInput;

char *ConsoleRead();
char ReadKey();
