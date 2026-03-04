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
  Key_Realising = 0x80, // отпускание клавиши
                        //  ----------------------------------------------
  KEY_MASK_ALT = 0xF5,
  KEY_MASK_SHIFT = 0xF4,
  KEY_MASK_CAPS =
      0xFF, // я хуй знает зачем эти гении придумали систему в которой их ps/2 и
            // ascii стандарты смешиваются и не дают жить друг друг причем блять
            // скан коды от клавы прилетают такие же 58 капс и 58 : что за еп
            // твою мать когда у меня все утыкано u8
            //  если без бреда то сканкоды PS/2 равны кодам в таблице ASCII
            //  (некоторых символах таких как 8,*,:)
};

u8 ReturnCharKeyboard(u8 sc);
