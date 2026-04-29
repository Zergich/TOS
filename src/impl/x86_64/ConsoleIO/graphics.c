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

  const u8 *glyph = &font8x16[(uint8_t)c * 16];

  for (int cy = 0; cy < 16; cy++) {
    // 2. Берем байт, описывающий 8 горизонтальных пикселей
    u8 row = glyph[cy];

    for (int cx = 0; cx < 8; cx++) {
      // 3. Проверяем биты слева направо
      if ((row >> (7 - cx)) & 1) {
        // Используйте Pitch / 4 для надежности
        BuffPtr[(y + cy) * WidthDisplay + (x + cx)] = color;
      }
    }
  }
}
