#pragma once
#include <types.h>

typedef struct {
  u16 position;
  int (*put)(char character);
  char *(*get)(char *linkchar);
  char *buffer;
} RoundBufferObgect;

int PutIntoBuffer(char character);

int GetgFromBuffer(char *linkchar);
