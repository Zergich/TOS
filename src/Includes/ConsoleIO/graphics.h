#pragma once

#include <types.h>

typedef struct {
  uint32_t CodePoint; // Номер символа Unicode (например, 0x0410 для 'А')
  uint32_t FgColor;   // Цвет текста
  uint32_t BgColor;   // Цвет фона
} TerminalCell;

typedef struct {
  u32 *(*Init)(u32 *framebuffer_ptr);
  void (*Draw)(u64 x, u64 y, u32 color);
  void (*DrawChar)(u64 x, u64 y, u32 c, u32 fg_color, u32 bg_color);
  u32 *ptr;
} Pixeling;
extern Pixeling PixelGrapchics;
u32 *InitDraw(u32 *framebuffer_ptr);
void DrawChar(u64 x, u64 y, u32 c, u32 fg_color, u32 bg_color);
void DrawPixel(u64 x, u64 y, u32 color);
u32 Uft8Decoder(char **string);
u32 FindGlyphIndex(u32 target_codepoint);
