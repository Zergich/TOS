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
u32 FindGlyphIndex(u32 target_codepoint) {
  if (current_font == NULL)
    return 0;

  // Если в шрифте нет таблицы Unicode, возвращаем сам код как индекс (как было
  // раньше)
  if ((current_font->flags & 1) == 0) {
    return target_codepoint < current_font->numglyph ? target_codepoint : 0;
  }

  // Высчитываем адрес начала таблицы Unicode.
  // Она начинается ровно там, где заканчиваются графические данные всех глифов.
  u8 *unicode_table = (u8 *)current_font + current_font->headersize +
                      (current_font->numglyph * current_font->bytesperglyph);

  u32 current_glyph_index = 0;
  char *ptr = (char *)unicode_table; // Указатель для бега по таблице

  while (current_glyph_index < current_font->numglyph) {
    // Байт 0xFF в PSF2 означает: "Коды для текущего глифа закончились,
    // переходим к следующему"
    if ((u8)(*ptr) == 0xFF) {
      current_glyph_index++;
      ptr++;
      continue;
    }

    // Байт 0xFE в PSF2 используется для составных символов (например, буква +
    // ударение). В рамках базового текста мы просто пропускаем этот управляющий
    // байт.
    if ((u8)(*ptr) == 0xFE) {
      ptr++;
      continue;
    }

    // Читаем код из таблицы шрифта с помощью ТВОЕГО декодера
    // Обрати внимание: Uft8Decoder сам сдвинет указатель ptr вперед!
    u32 table_codepoint = Uft8Decoder(&ptr);

    // Сравниваем то, что прочитали из таблицы, с тем, что мы ищем
    if (table_codepoint == target_codepoint) {
      return current_glyph_index; // Бинго! Нашли нужный индекс картинки.
    }
  }

  // Если мы прочесали всю таблицу и не нашли совпадений,
  // возвращаем 0 (в твоем шрифте это глиф пустого квадрата или домика)
  return 0;
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
    u8 next_byte = code[i];

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
