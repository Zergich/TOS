#pragma once

#include <types.h>

typedef struct {
  u32 *(*Init)(u32 *framebuffer_ptr);
  void (*Draw)(u64 x, u64 y, u32 color);
  void (*DrawChar)(u64 x, u64 y, char c, u32 fg_color, u32 bg_color);
  u32 *ptr;
} Pixeling;
extern Pixeling PixelGrapchics;
u32 *InitDraw(u32 *framebuffer_ptr);
void DrawChar(u64 x, u64 y, char c, u32 fg_color, u32 bg_color);
void DrawPixel(u64 x, u64 y, u32 color);
