#include <ConsoleIO/console.h>
#include <ConsoleIO/print.h>
#include <System/Array.h>
#include <System/keyboard.h>
#include <System/sysinfo.h>
#include <VGA/vgacursor.h>
#include <datastruct.h>
#include <stddef.h>
#include <string.h>
#include <types.h>

extern RoundBufferObgect RoundBuff;
extern StringStruct string;

extern u16 CursorPosCol;
extern u16 CursorPosRow;
extern size_t VGA_Column; // зачем это нужно описано в обработке клавишь

extern struct Char *buffer;

ConsoleInput Console = {.ReadLine = ConsoleRead, .ReadKey = ReadKey

};
u8 LimitXRow = 7;
u32 CarretIndex = 0; // фактическая позиция курсора (учитывает все строки)
u32 TextSize = 0;
static int max_len =
    StringLenght; // ну что могу сказать зеленый еще я и без статика все хуева
                  // нужны аллокаторы но где ты их блять возьмешь
void ShiftLeft() {
  if (CarretIndex <= 0 || TextSize <= 0)
    return;

  int base_offset = 80 * CursorPosRow;

  // Строка жестко привязана к этой колонке!
  int visual_offset = CursorPosCol - CarretIndex;

  for (int i = CarretIndex; i < TextSize; i++) {
    buffer[base_offset + visual_offset + i - 1] =
        buffer[base_offset + visual_offset + i];
  }

  int last_char_pos = base_offset + visual_offset + TextSize - 1;
  buffer[last_char_pos].character = ' ';
}

void BackSpaceHandle(char *string, u16 lastindex) {
  // лимит по X
  int _1 = CursorPosCol;
  int _2 = CursorPosRow;
  u16 lineralpos = LimitXRow + CarretIndex;
  // ConsoleSetCarretPos(0, 0);
  // printf("|%u|%u|%u|", lastindex, statlen(string1), TextSize);
  // ConsoleSetCarretPos(_1, _2);

  if (lineralpos == LimitXRow - 2) // без -2 не работает
    return;
  else if (lineralpos == 7)
    return;
  if (CursorPosCol == 0) {
    ConsoleSetCarretPos(80, CursorLine() - 1);
  }
  ShiftLeft();
  // CursorSetColumn(CursorColumn() - 1);
  // print(" ");
  // CursorSetColumn(CursorColumn() - 1);
  CursorPosCol -= 1; // из за того что функция принт тоже двигает курсор
  CursorPos(CursorPosCol, CursorPosRow);
  IndexDeleteC(string, &TextSize, lastindex);
  // printf("\n%u", lastindex);
  // string[lastindex] = 0;
}
void ArrowHandleRL(u8 ArrowType) // пока только право лево
{
  u16 lineralpos = LimitXRow + CarretIndex;
  if (CarretIndex == 0 && ArrowType == LeftArrow)
    return;
  if (lineralpos == LimitXRow && ArrowType != RightArrow)
    return;
  if (ArrowType == RightArrow && lineralpos == TextSize + LimitXRow)
    return;
  int Mover = 0; // хрень которая определяет в какую сторону пойдет курсор
  if (ArrowType == LeftArrow) {
    Mover = -1;
    CursorPosCol--;
    CarretIndex--;
  } else {
    Mover = 1;
    CursorPosCol++;
    CarretIndex++;
  }
  CursorSetColumn(CursorColumn() + Mover);
  CursorPos(CursorPosCol, CursorPosRow);
}

bool SpecCodeConsoleRead = false;
bool CheckSpecKeys(u8 SpecKey) {
  switch (SpecKey) { // enter обрабатывается отдельно в функции ниже
  case Key_Tab:
  case Key_LShift:
  case Key_RShift:
  case Key_Ctrl:
  case Key_Alt:
  case Key_CapsLock:
    // printf(" |%u|%h|%c|", SpecKey, SpecKey, SpecKey);
    return false;
  case RightArrow:
  case LeftArrow:
    ArrowHandleRL(SpecKey);
    break;
  case DownArrow:
  case UpArrow:
    return false;
  default:
    return true;
  }
  return false;
}
void ShiftRight() {
  if (CarretIndex >= TextSize)
    return;

  // я то мыслю правильно и пишу код в правильном направлении но так впадлу
  // додумавать всю эту хунйю с индексами сторонами и тд по этому нейронка в
  // этом случаех
  int base_offset = 80 * CursorPosRow;

  // ВЫЧИСЛЯЕМ СМЕЩЕНИЕ:
  // Физическая позиция курсора минус логическая позиция в строке.
  // Это даст нам колонку, с которой реально начинается текст на экране.
  int visual_offset = CursorPosCol - CarretIndex;

  // Идем с КОНЦА текста в НАЧАЛО.
  for (int i = TextSize - 1; i >= CarretIndex; i--) {
    // Читаем символ, учитывая РЕАЛЬНОЕ место на экране (base + смещение +
    // индекс)
    struct Char current = buffer[base_offset + visual_offset + i];

    // +1 сдвиг в право
    buffer[base_offset + visual_offset + i + 1] = current;
  }
}
int ConsoleRead(char *string) { // мб спипать спец коды и отсавлять только
                                // аски соответственно.
  // это понадобится для чтении клавиши, ведь при текущей
  // реализации читает только ascii без спец кодовх
  u32 i = 0;
  u8 c = 0;
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
      if (i == 0 || CarretIndex == 0)
        continue;
      BackSpaceHandle(string, CarretIndex - 1);
      CarretIndex--;
      VGA_Column =
          CursorPosCol; // из за того что я еблан и у меня в консоли 2 системы
                        // координат и после того как я мувнулся влево и удалил
                        // до конца координаты шлют меня нахуй и печать
                        // начинается с того момента когда я начал удалять а
                        // символ которрый остался в начале копируется. И опять
                        // без нейронки не обошлось, но в конце концов я оставил
                        // свой вариант
      continue;
    }

    // Клавиша Enter обычно посылает код 0x0D ('\r'), иногда 0x0A ('\n')
    if (c == '\r' || c == '\n') {
      PrintChar('\n');
      CarretIndex = 0;
      TextSize = 0;
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
    TextSize++;
    IndexInsertC(string, &i, max_len, CarretIndex++, c);
    // string[i++] = c;
    ShiftRight();
    PrintChar(c);
  }
  // IndexInsertC(string, &TextSize, max_len, i, '\0');
  string[i] = '\0'; // для корректного завершения строки

  return 0;
}

char ReadKey() {
  u8 GetChar;
  RoundBuff.get(&GetChar);
  if (GetChar) {
    return GetChar;
  }
  return -1;
}
void ConsoleBufferReadString(u8 Start, u8 End, u8 MaxColumn, u8 Line,
                             struct Char *ReadedBuffer) {
  // struct Char *buffer = (struct Char *)0xb8000; // VGA память

  for (u8 i = 0; i < End; i++)
    ReadedBuffer[i] = buffer[(Start + i) + MaxColumn * Line];
}
