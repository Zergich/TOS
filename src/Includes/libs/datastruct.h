#pragma once
#include <types.h>

typedef struct {
  int (*put)(u8 character);
  int (*get)(u8 *linkchar);
  u8 buffer[8192];
} RoundBufferObgect;

int PutIntoBuffer(u8 character);
int GetgFromBuffer(u8 *linkchar);
