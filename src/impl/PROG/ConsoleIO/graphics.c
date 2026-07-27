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
void DrawChar(u64 x, u64 y, char c, u32 fg_color, u32 bg_color) {
  x *= FONT_WIDTH;
  y *= FONT_HEIGHT;
  uint32_t char_index = (uint8_t)c;
  const u8 *glyph = &vga_font[(char_index * 16)];

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

typedef struct {
  uint32_t CodePoint; // Номер символа Unicode (например, 0x0410 для 'А')
  uint32_t FgColor;   // Цвет текста
  uint32_t BgColor;   // Цвет фона
} TerminalCell;

u32 Uft8Decoder(const char **string) // берем символ из строки
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
// Заменяешь printf на свой kprintf (только ASCII-символы!)
void test_utf8_decoder(void) {
  // Строка из байт:
  // '\x41'             -> 'A' (1 байт)
  // '\xD0\x9F'         -> 'П' (2 байта)
  // '\xE2\x95\x8C'     -> '┌' (3 байта)
  // '\xF0\x9F\x9A\x80' -> эмодзи (4 байта)
  const char test_bytes[] = "\x41"
                            "\xD0\x9F"
                            "\xE2\x94\x8C"
                            "\xF0\x9F\x9A\x80";
  const char *ptr = test_bytes;
  const char *prev_ptr;

  // Ожидаемые значения (Code Point и байтовый сдвиг)
  uint32_t expected_cp[] = {0x0041, 0x041F, 0x250C, 0x1F680};
  int expected_bytes[] = {1, 2, 3, 4};

  print("\n=== UTF8 DECODER TEST ===\n");

  for (int i = 0; i < 4; i++) {
    prev_ptr = ptr;
    uint32_t cp = Uft8Decoder(&ptr);
    int advanced = (int)(ptr - prev_ptr);

    printf("Test %u: CP=0x%h (exp 0x%h), ADV=%u (exp %u) -> ", i + 1, cp,
           expected_cp[i], advanced, expected_bytes[i]);

    if (cp == expected_cp[i] && advanced == expected_bytes[i]) {
      print("OK\n");
    } else {
      print("FAIL!\n");
      return;
    }
  }

  // Тест на битый байт (0xFF)
  const char bad_byte[] = "\xFF";
  ptr = bad_byte;
  uint32_t bad_cp = Uft8Decoder(&ptr);

  printf("Test 5 (Bad byte): CP=0x%h -> ", bad_cp);
  if (bad_cp == 0xFFFD) {
    print("OK\n");
  } else {
    print("FAIL!\n");
    return;
  }

  print("=== ALL TESTS PASSED ===\n\n");
}
