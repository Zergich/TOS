#pragma once

#include <types.h>

typedef struct {
  void (*UpTime)(void);
  void (*Sleep)(u32);
  volatile u32 PitTimerSecondsUp;
  volatile u64 PitTimerMiliSecondsUp;
} TimePit;

extern TimePit Timepit;
// void UpTime();
// void Sleep(u32);

// void pit_init(int);
