#include <System/keyboard.h>
#include <VGA/console.h>
#include <VGA/print.h>
#include <VGA/vgacursor.h>
#include <datastruct.h>
#include <string.h>
#include <types.h>

extern RoundBufferObgect RoundBuff;

extern size_t NUM_COLUMS;
extern size_t NUM_ROWS;

extern u16 CursorPosCol;
extern u16 CursorPosRow;

ConsoleInput Console = {.ReadLine = ConsoleRead, .ReadKey = ReadKey

};
u8 LimitXRow = 7;
void BackSpaceHandle(char *string, u16 lastindex) {
  // лимит по X
  if (CursorColumn() == LimitXRow)
    return;
  CursorSetColumn(CursorColumn() - 1);
  print(" ");
  CursorSetColumn(CursorColumn() - 1);
  CursorPosCol -= 2; // из за того что функция принт тоже двигает курсор
  CursorPos(CursorPosCol, CursorPosRow);
  string[lastindex] = 0;
}
void ArrowHandle(u8 ArrowType) // пока только право лево
{
  if (CursorColumn() == LimitXRow)
    return;
  CursorSetColumn(CursorColumn() - 1);
  CursorPos(--CursorPosCol, CursorPosRow);
}

bool SpecCodeConsoleRead = false;
bool CheckSpecKeys(u8 SpecKey) {
  switch (SpecKey) { // enter обрабатывается отдельно в функции ниже
  case Key_Tab:
    print("pede");
    break;
  case Key_LShift:
  case Key_RShift:
  case Key_Ctrl:
  case Key_Alt:
  case Key_CapsLock:
    // printf(" |%u|%h|%c|", SpecKey, SpecKey, SpecKey);
    return false;
  case UpArrow:
  case RightArrow:
  case LeftArrow:
  case DownArrow:
    return false;
  default:
    return true;
  }
}

int ConsoleRead(char *string) { // мб спипать спец коды и отсавлять только
                                // аски соответственно.
  // это понадобится для чтении клавиши, ведь при текущей
  // реализации читает только ascii без спец кодовх
  uint16_t i = 0;
  u8 c = 0;
  static int max_len =
      StringLenght; // ну что могу сказать зеленый еще я и без статика все хуева
                    // нужны аллокаторы но где ты их блять возьмешь
  while (i < max_len - 1) {
    if (RoundBuff.get(&c) != 0) {
      // Буфер пуст (клавишу еще не нажали)
      // Вместо бесконечного нагрузочного цикла ставим процессор на паузу
      // (hlt). Он проснется только когда придет прерывание (нажатие
      // клавиши).
      asm volatile("hlt");
      continue;
    }
    if (c == '\b') {
      if (i == 0) // иначе цикл завершится
        continue;
      BackSpaceHandle(string, --i);
      continue;
    }

    // Клавиша Enter обычно посылает код 0x0D ('\r'), иногда 0x0A ('\n')
    if (c == '\r' || c == '\n') {
      PrintChar('\n'); // Перевести строку на экране для красоты
      break;
    }
    if (SpecCodeConsoleRead) {
      CheckSpecKeys(c);
      SpecCodeConsoleRead = false;
      continue;
    }
    if (c == Code_MagickCode) {
      SpecCodeConsoleRead = true;
      continue;
    }
    string[i++] = c;
    PrintChar(c);
  }
  string[i] = '\0'; // для корректного завершения строки

  return 0;
}
char ReadKey() {
  char GetChar;
  RoundBuff.get(&GetChar);
  if (GetChar) {
    return GetChar;
  }
  return -1;
}
