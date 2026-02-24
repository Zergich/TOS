#include <keyboard.h>
#include <stdint.h>
#include <types.h>
// стартовые позиции курсора
uint16_t CursorPosCol = 7;
uint16_t CursorPosRow = 3;

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

// Простой маппинг кода клавиши на ASCII для букв и цифр
bool ShiftEnabled = false;
bool CapsEnabled = false;
char ReturnCharKeyboard(uint8_t sc) {
  // Минимальный набор (коды без расширенных, без Shift)
  if (sc == Key_CapsLock) {
    if (CapsEnabled)
      CapsEnabled = false;
    else
      CapsEnabled = true;
    return sc;
  }

  if (sc == Key_LShift || sc == Key_RShift) {
    ShiftEnabled = true;
    return sc;
  } else if (sc == (Key_LShift | Key_Realising) ||
             sc == (Key_RShift | Key_Realising)) {
    ShiftEnabled = false;
    return sc;
  } else {
    static const char MapLow[256] = {
        [0x1E] = 'a', [0x30] = 'b',  [0x2E] = 'c', [0x20] = 'd', [0x12] = 'e',
        [0x21] = 'f', [0x22] = 'g',  [0x23] = 'h', [0x17] = 'i', [0x24] = 'j',
        [0x25] = 'k', [0x26] = 'l',  [0x32] = 'm', [0x31] = 'n', [0x18] = 'o',
        [0x19] = 'p', [0x10] = 'q',  [0x13] = 'r', [0x1F] = 's', [0x14] = 't',
        [0x16] = 'u', [0x2F] = 'v',  [0x11] = 'w', [0x2D] = 'x', [0x15] = 'y',
        [0x2C] = 'z', [0x0B] = '0',  [0x02] = '1', [0x03] = '2', [0x04] = '3',
        [0x05] = '4', [0x06] = '5',  [0x07] = '6', [0x08] = '7', [0x09] = '8',
        [0x0A] = '9', [0x1C] = '\n', [0x39] = ' ',
        // Можно расширить таблицу при необходимости
    };
    char ch = MapLow[sc];
    // если зажат шифт то Заглавные. Если активирован капс то заглавнае. Если
    // активирован капс И зажат шифт то маленькие или спец.
    if ((CapsEnabled && !ShiftEnabled) || (!CapsEnabled && ShiftEnabled)) {
      if ((ShiftEnabled || CapsEnabled) && ch >= 'a' && ch <= 'z') {
        ch = ch - ('a' - 'A'); // переводим в верхний регистр
      }
    }
    if (sc & Key_Realising)
      return 0; // отпускание клавиши - игнорируем
    return ch;
  }
}
