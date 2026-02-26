#pragma once
#include <types.h>

typedef struct {
  int (*put)(char character);
  int (*get)(char *linkchar);
  char buffer[8192];
} RoundBufferObgect;

int PutIntoBuffer(char character);

int GetgFromBuffer(char *linkchar);
char *readline(char *buffer, uint16_t max_len);
