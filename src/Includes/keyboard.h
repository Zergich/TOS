#pragma once
#include <stdint.h>

enum SpecKodes {
  Key_Backspace = 0x0E,
  Key_Tab = 0x0F,
  Key_Enter = 0x1C,
  Key_LShift = 0x2A,
  Key_RShift = 0x36,
  Key_Ctrl = 0x1D,
  Key_Alt = 0x38,
  Key_CapsLock = 0x3A,
  Key_Realising = 0x80, // отпускание клавиши
};

char ReturnCharKeyboard(uint8_t sc);
