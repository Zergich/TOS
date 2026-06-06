#pragma once
#include <types.h>

struct WindowInfo {
  u16 Height;
  u16 Width;

  u16 PosX;
  u16 PosY;
};

void DrawsQare(struct WindowInfo info);
