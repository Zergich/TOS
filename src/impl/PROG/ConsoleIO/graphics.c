#include <ConsoleIO/font.h>
#include <ConsoleIO/graphics.h>
#include <System/sysinfo.h>
#include <stddef.h>
#include <types.h>

#include <ConsoleIO/print.h>

u32 *BuffPtr = 0;
Pixeling PixelGrapchics = {

    .Init = InitDraw,
    .Draw = DrawPixel,
    .DrawChar = DrawChar,
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

void DrawChar(u64 x, u64 y, u32 c, u32 fg_color, u32 bg_color) {
  if (current_font == NULL)
    return;

  x *= FONT_WIDTH;
  y *= FONT_HEIGHT;
  u32 glyph_index = c;

  if (glyph_index >= current_font->numglyph) {

    glyph_index = 0;
  }
  // эти ебанные дауны не могли разобраться в 3 переменных и в итоге проебал
  // почти день когда надо было просто переставить пару переменных в старую
  // функцию

  // Вычисляем, сколько байт занимает ОДНА строка пикселей символа
  // (например, для шрифта шириной 8 это 1 байт, для 16 - 2 байта)
  u32 bytes_per_line = current_font->bytesperglyph / current_font->height;
  // Высчитываем адрес нужного символа
  u8 *glyph = (u8 *)current_font + current_font->headersize +

              (glyph_index * current_font->bytesperglyph);

  // const u8 *glyph = &vga_font[(c * 16)];

  u8 Bold = 1;     // стандарт 1
  u8 Interval = 7; // стандарт 7

  for (int cy = 0; cy < FONT_HEIGHT; cy++) {
    u8 row = glyph[cy];
    for (int cx = 0; cx < FONT_WIDTH; cx++) {
      size_t offset = (y + cy) * BufferPitchW + (x + cx);
      if ((row >> (Interval - cx)) & Bold) {
        BuffPtr[offset] = fg_color; // рисование символов
      } else {
        BuffPtr[offset] = bg_color; // затирка фона если будет пробел и тп
      }
    }
  }
}

u32 Uft8Decoder(char **string) // берем символ из строки
{
  const u8 *code = (const u8 *)*string; // конвертер в u8 символ
  u32 CodePoint = 0;
  int BytesToRead = 0;
  u8 FirstByte = *code; // определить к какйо кодировке относится символ

  if ((FirstByte & 0x80) == 0x00) {
    CodePoint = FirstByte;
    BytesToRead = 1;
  } else if ((FirstByte & 0xE0) == 0xC0) {
    CodePoint = FirstByte & 0x1F; // вырезаем 5 последних бит
    BytesToRead = 2;
  } else if ((FirstByte & 0xF0) == 0xE0) {
    // 3 байта: 1110xxxx (здесь лежат рамки TUI)
    CodePoint = FirstByte & 0x0F; // Вырезаем 4 полезных бита
    BytesToRead = 3;
  } else if ((FirstByte & 0xF8) == 0xF0) {
    // 4 байта: 11110xxx
    CodePoint = FirstByte & 0x07; // Вырезаем 3 полезных бита
    BytesToRead = 4;
  } else {
    // Ошибка: невалидный байт. Возвращаем символ замены ()
    *string += 1;
    return 0xFFFD;
  }

  for (int i = 1; i < BytesToRead; i++) {
    uint8_t next_byte = code[i];

    // Проверяем, что байт-продолжение действительно начинается с '10'
    if ((next_byte & 0xC0) != 0x80) {
      *string += 1;
      return 0xFFFD; // Битая последовательность
    }

    // Сдвигаем то, что уже накопили, на 6 бит влево
    // и приклеиваем 6 полезных бит из нового байта
    CodePoint = (CodePoint << 6) | (next_byte & 0x3F);
  }

  // 4. Двигаем оригинальный указатель вперед
  *string += BytesToRead;
  return CodePoint;
}
