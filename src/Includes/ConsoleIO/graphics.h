#pragma once

#include <types.h>

typedef struct {
  int (*Init)(u32 *framebuffer_ptr);
  void (*Draw)(u64 x, u64 y, u32 color);
  void (*DrawChar)(u64 x, u64 y, char c, u32 color);

} Pixeling;

int InitDraw(u32 *framebuffer_ptr);
void DrawPixel(u64 x, u64 y, u32 color);
void DrawChar(u64 x, u64 y, char c, u32 color);
