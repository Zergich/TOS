#include <ConsoleIO/font.h>
#include <ConsoleIO/graphics.h>
#include <System/sysinfo.h>
#include <types.h>

Pixeling PixelGrapchics = {

    .Init = InitDraw,
    .Draw = DrawPixel,
    .DrawChar = DrawChar,
};

u32 *BuffPtr = 0;

int InitDraw(u32 *framebuffer_ptr) {
  BuffPtr = framebuffer_ptr;
  return 0;
}
void DrawPixel(u64 x, u64 y, u32 color) {
  for (; x < y; x++) {
    BuffPtr[x] = color;
  }
}

void DrawChar(u64 x, u64 y, char c, u32 color) {

  uint32_t char_index = (uint8_t)c;
  const u8 *glyph = &vga_font[(char_index * 16)];

  for (int cy = 0; cy < 16; cy++) {
    u8 row = glyph[cy];
    for (int cx = 0; cx < 8; cx++) {
      // Попробуйте (row & (0x80 >> cx)) - это классический порядок (MSB first)
      if ((row >> (7 - cx)) & 1) {
        // Используйте Pitch / 4. В Limine это критично!
        size_t offset = (y + cy) * WidthDisplay + (x + cx);
        BuffPtr[offset] = color;
      }
    }
  }
}
