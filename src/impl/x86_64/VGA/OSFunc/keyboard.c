#include <keyboard.h>
#include <print.h>
#include <stdint.h>
#include <types.h>
// стартовые позиции курсора
uint16_t CursorPosCol = 7;
uint16_t CursorPosRow = 3;

// Простой маппинг кода клавиши на ASCII для букв и цифр
bool ShiftEnabled = false;
bool CapsEnabled = false;
u8 ReturnCharKeyboard(u8 sc) {
  // Минимальный набор (коды без расширенных, без Shift)
  if (sc == Key_Alt)
    return KEY_MASK_ALT;
  if (sc == Key_CapsLock) {
    if (CapsEnabled)
      CapsEnabled = false;
    else
      CapsEnabled = true;
    return KEY_MASK_CAPS; // маска от коллизий между аски таблицой
  }

  if (sc == Key_LShift || sc == Key_RShift) {
    ShiftEnabled = true;
    return KEY_MASK_SHIFT;
  } else if (sc == (Key_LShift | Key_Realising) ||
             sc == (Key_RShift | Key_Realising)) {
    ShiftEnabled = false;
    return 0;
  } else {
    static const char MapLow[256] = {
        [0x1E] = 'a', [0x30] = 'b',  [0x2E] = 'c',  [0x20] = 'd', [0x12] = 'e',
        [0x21] = 'f', [0x22] = 'g',  [0x23] = 'h',  [0x17] = 'i', [0x24] = 'j',
        [0x25] = 'k', [0x26] = 'l',  [0x32] = 'm',  [0x31] = 'n', [0x18] = 'o',
        [0x19] = 'p', [0x10] = 'q',  [0x13] = 'r',  [0x1F] = 's', [0x14] = 't',
        [0x16] = 'u', [0x2F] = 'v',  [0x11] = 'w',  [0x2D] = 'x', [0x15] = 'y',
        [0x2C] = 'z', [0x0B] = '0',  [0x02] = '1',  [0x03] = '2', [0x04] = '3',
        [0x05] = '4', [0x06] = '5',  [0x07] = '6',  [0x08] = '7', [0x09] = '8',
        [0x0A] = '9', [0x1C] = '\n', [0x39] = ' ',  [0x1A] = '[', [0x1B] = ']',
        [0x33] = ',', [0x34] = '.',  [0x35] = '/',  [0x27] = ';', [0x28] = '\'',
        [0x29] = '`', [0x2B] = '\\', [0x0F] = '\t', [0x0C] = '-', [0x0D] = '=',
    };
    static const char MapShift[97] = {
        [0x02] = '!', [0x03] = '@', [0x04] = '#', [0x05] = '$',  [0x06] = '%',
        [0x07] = '^', [0x08] = '&', [0x09] = '*', [0x0A] = '(',  [0x0B] = ')',
        [0x0C] = '_', [0x0D] = '+', [0x1A] = '{', [0x01B] = '}', [0x2B] = '|',
        [0x33] = '<', [0x34] = '>', [0x35] = '?', [0x29] = '~',  [0x27] = ':',
        [0x28] = '"'};

    char ch = MapLow[sc];
    char ShiftKey = MapShift[sc];
    // если зажат шифт то Заглавные. Если активирован капс то заглавнае. Если
    // активирован капс И зажат шифт то маленькие или спец.

    if ((CapsEnabled && !ShiftEnabled) || (!CapsEnabled && ShiftEnabled)) {
      if ((ShiftEnabled || CapsEnabled) && ch >= 'a' && ch <= 'z') {
        ch = ch - ('a' - 'A'); // переводим в верхний регистр
      }
    }
    if (sc & Key_Realising)
      return 0; // отпускание клавиши - игнорируем
    if (ShiftEnabled && ShiftKey != 0) {
      return ShiftKey;
    }
    return ch;
  }
}
