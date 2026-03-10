#pragma once

#include <types.h>

typedef struct {
  void (*UpTime)(void);
  void (*Sleep)(u32);
  int (*TickEvent)(int);
  volatile u32 PitTimerSecondsUp;
} TimePit;

void UpTime();
int PitTickEvent();
void Sleep(u32);
