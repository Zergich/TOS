#pragma once
#include <types.h>

typedef struct {
  int (*put)(u32 character);
  int (*get)(u32 *linkchar);
  u32 buffer[8192];
} RoundBufferObgect;

int PutIntoBuffer(u32 character);
int GetgFromBuffer(u32 *linkchar);
