
#include "io.h"
#include <interrupts.h>
#include <shell.h>

#define SC_MAX 58 // Increased from 57 to 58 to match array size

static int shift_pressed = 0;

const char normal_map[SC_MAX] = {
    0,   0,   '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
    '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
    'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
    'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm',  ',',  '.',  '/', 0,   '*',  0,   ' '};

const char shift_map[SC_MAX] = {
    0,   0,   '!',  '@',  '#',  '$', '%', '^', '&', '*', '(', ')',
    '_', '+', '\b', '\t', 'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '{',  '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H',
    'J', 'K', 'L',  ':',  '"',  '~', 0,   '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M',  '<',  '>',  '?', 0,   '*', 0,   ' '};

void interrupt_handler_keyboard(void) {
  unsigned char scancode = inb(0x60);

  // Handle Shift press and release
  if (scancode == 0x2A || scancode == 0x36) { // Left or Right Shift press
    shift_pressed = 1;
    pic_ack(1);
    return;
  } else if (scancode == 0xAA ||
             scancode == 0xB6) { // Left or Right Shift release
    shift_pressed = 0;
    pic_ack(1);
    return;
  }

  // Only process non-release scancodes (< 0x80)
  if (scancode < SC_MAX && scancode < 0x80) {
    char c = shift_pressed ? shift_map[scancode] : normal_map[scancode];
    if (c) {
      shell_handle_input(c);
    }
  }
  pic_ack(1);
}
