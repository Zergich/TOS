#pragma once
#include <stdint.h>
#include <types.h>
enum SpecKodes {
  Key_Backspace = 0x0E,
  Key_Tab = 0x0F,
  Key_Enter = 0x1C,
  Key_LShift = 0x2A,
  Key_RShift = 0x36,
  Key_Ctrl = 0x1D,
  Key_Alt = 0x38,
  Key_CapsLock = 0x3A,
  Key_Realising = 0x80,   // отпускание клавиши
  Code_MagickCode = 0xFF, // для обозначения что следующий код надо обработать
                          // как спец код а не символ аски
  Code_KeySpec =
      0xE0, // этот код означает что после него пойдет другой который уже
            // отвечает за стрекли инсерты и тд (чтоб коллизиц не былох)
  RightArrow = 0x4D,
  LeftArrow = 0x4B,
  DownArrow = 0x50,
  UpArrow = 0x48,

};

u8 ReturnCharKeyboard(u8 sc);
