#pragma once

#include <type.h>

typedef struct {
  void (*UpTime)(void);
  void (*Sleep)(u32);
} TimePit;

void UpTime();
void Sleep(u32);
