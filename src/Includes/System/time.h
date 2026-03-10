#pragma once

#include <types.h>

typedef struct {
  void (*UpTime)(void);
  void (*Sleep)(u32);
  volatile u32 PitTimerSecondsUp;
} TimePit;

void UpTime();
void Sleep(u32);
