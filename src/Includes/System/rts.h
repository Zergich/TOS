#pragma once
#include <types.h>

struct DateTime {
  u8 second;
  u8 minute;
  u8 hour;
  u8 day;
  u8 month;
  u32 year;
};

struct DateTime GetTimeRTS();
