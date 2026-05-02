#include <ConsoleIO/font.h>
#include <ConsoleIO/graphics.h>
#include <System/sysinfo.h>
#include <types.h>

u32 *BuffPtr = 0;
Pixeling PixelGrapchics = {

    .Init = InitDraw,
    .Draw = DrawPixel,
    .DrawChar = DrawChar,
    .DrawCharOf = DrawCharOf,
    .ptr = 0,
};

u32 *InitDraw(u32 *framebuffer_ptr) {
  BuffPtr = framebuffer_ptr;
  PixelGrapchics.ptr = framebuffer_ptr;
  return BuffPtr;
}
void DrawPixel(u64 x, u64 y, u32 color) {
  for (; x < y; x++) {
    BuffPtr[x] = color;
  }
}

void DrawCharOf(u64 offset, char c, u32 fg_color, u32 bg_color) {

  uint32_t char_index = (uint8_t)c;
  const u8 *glyph = &vga_font[(char_index * 16)];

  for (int cy = 0; cy < 16; cy++) {
    u8 row = glyph[cy];
    for (int cx = 0; cx < 8; cx++) {
      if ((row >> (7 - cx)) & 1) {
        BuffPtr[offset] = fg_color; // рисование символов
      } else {
        BuffPtr[offset] = bg_color; // затирка фона если будет пробел и тп
      }
    }
  }
}

void DrawChar(u64 x, u64 y, char c, u32 fg_color, u32 bg_color) {

  uint32_t char_index = (uint8_t)c;
  const u8 *glyph = &vga_font[(char_index * 16)];

  for (int cy = 0; cy < 16; cy++) {
    u8 row = glyph[cy];
    for (int cx = 0; cx < 8; cx++) {
      size_t offset = (y + cy) * BufferPitchW + (x + cx);
      if ((row >> (7 - cx)) & 1) {
        BuffPtr[offset] = fg_color; // рисование символов
      } else {
        BuffPtr[offset] = bg_color; // затирка фона если будет пробел и тп
      }
    }
  }
}
