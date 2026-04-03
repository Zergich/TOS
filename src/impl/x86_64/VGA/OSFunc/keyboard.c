#include <System/keyboard.h>
#include <VGA/print.h>
#include <datastruct.h>
#include <stdint.h>
#include <types.h>
// стартовые позиции курсора
uint16_t CursorPosCol = 7;
uint16_t CursorPosRow = 3;

// Простой маппинг кода клавиши на ASCII для букв и цифр
bool ShiftEnabled = false;
bool CapsEnabled = false;
bool SpecCode = false;

u8 ReturnSpecCodes(u8 ScanCode) { // шифт альт капс и тд стрелки отдельно
  switch (ScanCode) { // обработка шифта капса альта и других клавищь не
                      // трубующих магического байта
  case Key_Alt:
  case Key_Tab:
  case UpArrow:
  case RightArrow:
  case LeftArrow:
  case DownArrow:
    return ScanCode;
  case Key_CapsLock:
    if (CapsEnabled)
      CapsEnabled = false;
    else
      CapsEnabled = true;
    return ScanCode;
  case Key_LShift:
  case Key_RShift:
    ShiftEnabled = true;
    u8 ReturnCodePress = (ScanCode == Key_LShift)
                             ? Key_LShift
                             : Key_RShift; // задел на будущие для хуков
    return ReturnCodePress;
  case (Key_LShift | Key_Realising): // отжимание клавиши
  case (Key_RShift | Key_Realising):
    ShiftEnabled = false;
    u8 ReturnCodeUnPress =
        (ScanCode == (Key_LShift | Key_Realising))
            ? (Key_LShift | Key_Realising)
            : (Key_RShift | Key_Realising); // задел на будущие для хуков
    return ReturnCodeUnPress;
  }

  return 0;
}
extern RoundBufferObgect RoundBuff;
u8 LastKeyCode = 0;

u8 DoubleSpeckeyBoard(u8 sc) { return 0; }

u8 ReturnCharKeyboard(u8 sc) {
  // эта пиздагрязь просто так не работает чтоб без этих
  // костылей
  if (sc == 0xE0) // пропуск магического байта для клавишь
    return 0;
  if (SpecCode) { // спец клавиши по типу шифта энтера и тд не
                  // нам пад и не
    // фки
    RoundBuff.put(LastKeyCode);
    SpecCode = false;
  }
  u8 GetSpecKeyCode = ReturnSpecCodes(sc);
  LastKeyCode = GetSpecKeyCode;
  if (GetSpecKeyCode != 0) // чтоб не было коллизий
  {
    SpecCode = true;
    return Code_MagickCode;
  }
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
      [0x0E] = '\b'};
  static const char MapShift[256] = {
      [0x02] = '!', [0x03] = '@', [0x04] = '#', [0x05] = '$', [0x06] = '%',
      [0x07] = '^', [0x08] = '&', [0x09] = '*', [0x0A] = '(', [0x0B] = ')',
      [0x0C] = '_', [0x0D] = '+', [0x1A] = '{', [0x1B] = '}', [0x2B] = '|',
      [0x33] = '<', [0x34] = '>', [0x35] = '?', [0x29] = '~', [0x27] = ':',
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
