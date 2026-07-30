#pragma once

#include <types.h>

#define PSF2_MAGIC 0x864AB572
typedef struct {
  u32 magic;         // Магическое число (0x864AB572)
  u32 version;       // Версия (обычно 0)
  u32 headersize;    // Размер заголовка (обычно 32 байта)
  u32 flags;         // Флаги (есть ли таблица Unicode)
  u32 numglyph;      // Количество символов (обычно 256 или 512)
  u32 bytesperglyph; // Размер одного символа в байтах
  u32 height;        // Высота в пикселях (например, 16)
  u32 width;         // Ширина в пикселях (например, 8)
} psf2_header_t;
void TestFont();
// Глобальный указатель на наш активный шрифт
extern psf2_header_t *current_font;

extern u8 FONT_HEIGHT;
extern u8 FONT_WIDTH;
extern u8 vga_font[256 * 16];
