#include <System/io.h>
#include <VGA/vgacursor.h>
#include <stdint.h>

#define VGA_CTRL 0x3D4
#define VGA_DATA 0x3D5
#define CURSOR_HIGH 0x0E
#define CURSOR_LOW 0x0F

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static uint16_t CursorPosRow = 0;
static uint16_t CursorPosCol = 0;

void CursoreSize(uint8_t end_line) {
  outb(0x3D4, 0x0A);     // регистр start
  outb(0x3D5, 0x00);     // включаем бит 5
  outb(0x3D4, 0x0B);     // регистр end
  outb(0x3D5, end_line); // сканлиния - это слои пикселей символа макс 16 больше
                         // 16 ошибки нет 0 не отключает курсор
}
void CursorPos(uint16_t Column, uint16_t Row) {
  uint16_t pos = Row * VGA_WIDTH + Column;
  // старший байт
  outb(VGA_CTRL, CURSOR_HIGH);
  outb(VGA_DATA, (pos >> 8) & 0xFF);
  // младший байт
  outb(VGA_CTRL, CURSOR_LOW);
  outb(VGA_DATA, pos & 0xFF);
  CursoreSize(15);
}

void CursorHide() {
  outb(VGA_CTRL, 0x0A);
  uint8_t crt_control = inb(VGA_DATA);
  crt_control |= 0x20; // бит 5 — скрыть курсор
  outb(VGA_DATA, crt_control);
}

void CursorShow() {
  outb(VGA_CTRL, 0x0A);
  uint8_t crt_control = inb(VGA_DATA);
  crt_control &= ~0x20; // убрать бит 5
  outb(VGA_DATA, crt_control);
}
