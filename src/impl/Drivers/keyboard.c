#include <ConsoleIO/graphics.h>
#include <ConsoleIO/print.h>
#include <drivers/keyboard.h>
#include <libs/datastruct.h>
#include <libs/string.h>
#include <stdint.h>
#include <types.h>
// стартовые позиции курсора
// потому что shell> с пробелом

// Простой маппинг кода клавиши на ASCII для букв и цифр
bool ShiftEnabled = false;
bool CapsEnabled = false;
bool AltEnabled = false;
bool SpecCode = false;
bool magic = false;
u32 *Layout = U"EN";

u8 ReturnSpecCodes(u8 ScanCode) { // шифт альт капс и тд стрелки отдельно
  switch (ScanCode) { // обработка шифта капса альта и других клавищь не
                      // трубующих магического байта
  case Key_Alt:
    if (AltEnabled)
      AltEnabled = false;
    else
      AltEnabled = true;
    if (ShiftEnabled)
      Layout = (string.Strcmp(Layout, U"EN") == 0) ? U"RU" : U"EN";
    return ScanCode;

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
    if (AltEnabled)
      Layout = (string.Strcmp(Layout, U"EN") == 0) ? U"RU" : U"EN";
    return ReturnCodeUnPress;
  }

  return 0;
}
extern RoundBufferObgect RoundBuff;
u8 LastKeyCode = 0;
// Сначала в буфер попадает Code_MagickCode.
// Ваш ConsoleRead считывает его, выставляет флаг SpecCodeConsoleRead и ждет
// следующего байта. Но "следующий байт" (реальный код стрелки) попадет в буфер
// только при следующем прерывании клавиатуры. Если вы просто "тапаете" клавишу
// (нажали-отпустили), следующим прерыванием будет код "Отпускание клавиши"
// (Break code). В итоге: Событие нажатия обрабатывается только когда приходит
// событие отпускания. Это и есть ваша "небольшая задержка". Для обычных клавиш
// (букв) все происходит сразу: MapLow[sc] -> возврат -> запись в буфер. Поэтому
// они работают мгновенно.

static const u32 *MapLow;
static const u32 *MapShift;
void LayoutSwitcher(u32 *layout);
u32 ReturnCharKeyboard(u8 sc) {
  // эта пиздагрязь просто так не работает чтоб без этих
  // костылей
  if (sc == 0xE0) // пропуск магического байта для клавишь
  {
    magic = true;
    return 0;
  }
  u8 GetSpecKeyCode = ReturnSpecCodes(sc);
  LastKeyCode = GetSpecKeyCode;
  if (GetSpecKeyCode != 0) // чтоб не было коллизий
  // Огромный комментарий относится к этому месту
  // ну ведь можно ыбло посидеь подумсать, самостоятельно к этому придти
  {
    SpecCode = true;
    RoundBuff.put(Code_MagickCode);
    RoundBuff.put(GetSpecKeyCode);
    return 0;
  }
  LayoutSwitcher(Layout);
  u32 ch = ShiftEnabled ? MapShift[sc] : MapLow[sc];

  // ШАГ 2: Если Caps включен — он ТУПО инвертирует регистр БУКВ.
  // Ему плевать на Shift, он просто делает большую букву маленькой, а маленькую
  // — большой.
  if (CapsEnabled) {
    // --- АНГЛИЙСКИЙ ---
    if (ch >= 'A' && ch <= 'Z') {
      ch += 32; // Заглавную (пришедшую из MapShift) превращает в МАЛЕНЬКУЮ
    } else if (ch >= 'a' && ch <= 'z') {
      ch -= 32; // Строчную (пришедшую из MapNormal) превращает в БОЛЬШУЮ
    }

    // --- РУССКИЙ ---
    else if (ch >= 0x0410 && ch <= 0x042F) { // 'А' .. 'Я'
      ch += 32; // Заглавную превращает в МАЛЕНЬКУЮ 'а' .. 'я'
    } else if (ch >= 0x0430 && ch <= 0x044F) { // 'а' .. 'я'
      ch -= 32; // Строчную превращает в БОЛЬШУЮ 'А' .. 'Я'
    }

    // --- БУКВА Ё ---
    else if (ch == 0x0401) { // 'Ё' -> 'ё'
      ch = 0x0451;
    } else if (ch == 0x0451) { // 'ё' -> 'Ё'
      ch = 0x0401;
    }
  }
  if (sc & Key_Realising)
    return 0; // отпускание клавиши - игнорируем

  return ch;
}

void LayoutSwitcher(u32 *layout) {
  if (string.Strcmp(layout, U"RU") == 0) {
    MapLow = MapRusLowRU;
    MapShift = MapRusShiftRU;
  }
  if (string.Strcmp(layout, U"EN") == 0) {
    MapLow = MapLowEN;
    MapShift = MapShiftEN;
  }
}

const u32 MapLowEN[256] = {
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
const u32 MapShiftEN[256] = {
    [0x02] = '!', [0x03] = '@', [0x04] = '#', [0x05] = '$', [0x06] = '%',
    [0x07] = '^', [0x08] = '&', [0x09] = '*', [0x0A] = '(', [0x0B] = ')',
    [0x0C] = '_', [0x0D] = '+', [0x1A] = '{', [0x1B] = '}', [0x2B] = '|',
    [0x33] = '<', [0x34] = '>', [0x35] = '?', [0x29] = '~', [0x27] = ':',
    [0x28] = '"'};

// Русская раскладка (обычный режим)
const u32 MapRusLowRU[256] = {
    [0x1E] = 0x0444, [0x30] = 0x0438, [0x2E] = 0x0441, [0x20] = 0x0432,
    [0x12] = 0x0443, [0x21] = 0x0430, [0x22] = 0x043F, [0x23] = 0x0440,
    [0x17] = 0x0448, [0x24] = 0x043E, [0x25] = 0x043B, [0x26] = 0x0434,
    [0x32] = 0x044C, [0x31] = 0x0442, [0x18] = 0x0449, [0x19] = 0x0437,
    [0x10] = 0x0439, [0x13] = 0x043A, [0x1F] = 0x044B, [0x14] = 0x0435,
    [0x16] = 0x0433, [0x2F] = 0x043C, [0x11] = 0x0446, [0x2D] = 0x0447,
    [0x15] = 0x043D, [0x2C] = 0x044F, [0x1A] = 0x0445, [0x1B] = 0x044A,
    [0x33] = 0x0431, [0x34] = 0x044E, [0x29] = 0x0451, [0x27] = 0x0436,
    [0x28] = 0x044D, [0x0B] = '0',    [0x02] = '1',    [0x03] = '2',
    [0x04] = '3',    [0x05] = '4',    [0x06] = '5',    [0x07] = '6',
    [0x08] = '7',    [0x09] = '8',    [0x0A] = '9',    [0x1C] = '\n',
    [0x39] = ' ',    [0x0F] = '\t',   [0x0C] = '-',    [0x0D] = '=',
    [0x0E] = '\b',   [0x2B] = '\\',   [0x35] = '.'};

// Русская раскладка (режим Shift)
const u32 MapRusShiftRU[256] = {
    [0x1E] = 0x0424, [0x30] = 0x0418, [0x2E] = 0x0421, [0x20] = 0x0412,
    [0x12] = 0x0423, [0x21] = 0x0410, [0x22] = 0x041F, [0x23] = 0x0420,
    [0x17] = 0x0428, [0x24] = 0x041E, [0x25] = 0x041B, [0x26] = 0x0414,
    [0x32] = 0x042C, [0x31] = 0x0422, [0x18] = 0x0429, [0x19] = 0x0417,
    [0x10] = 0x0419, [0x13] = 0x041A, [0x1F] = 0x042B, [0x14] = 0x0415,
    [0x16] = 0x0413, [0x2F] = 0x041C, [0x11] = 0x0426, [0x2D] = 0x0427,
    [0x15] = 0x041D, [0x2C] = 0x042F, [0x1A] = 0x0425, [0x1B] = 0x042A,
    [0x33] = 0x0411, [0x34] = 0x042E, [0x29] = 0x0401, [0x27] = 0x0416,
    [0x28] = 0x042D, [0x02] = '!',    [0x03] = '"',    [0x04] = 0x2116,
    [0x05] = ';',    [0x06] = '%',    [0x07] = ':',    [0x08] = '?',
    [0x09] = '*',    [0x0A] = '(',    [0x0B] = ')',    [0x0C] = '_',
    [0x0D] = '+',    [0x2B] = '/',    [0x35] = ','};
