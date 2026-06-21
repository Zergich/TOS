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


struct DateTime {
  u8 second;
  u8 minute;
  u8 hour;
  u8 day;
  u8 month;
  u32 year;
};

struct DateTime GetTimeRTS();
