#include <ConsoleIO/console.h>
#include <ConsoleIO/font.h>
#include <ConsoleIO/graphics.h>
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
extern size_t Current_Column; // зачем это нужно описано в обработке клавишь

extern Pixeling PixelGrapchics;
ConsoleInput Console = {.ReadLine = ConsoleRead, .ReadKey = ReadKey

};
u8 LimitXRow = 7;
u32 CarretIndex = 0; // фактическая позиция курсора (учитывает все строки)
u32 TextSize = 0;
static int max_len =
    StringLenght; // ну что могу сказать зеленый еще я и без статика все хуева
                  // нужны аллокаторы но где ты их блять возьмешь

// Функция ТОЛЬКО для работы с памятью и экраном. Без изменения глобальных
// курсоров.
void ShiftLeft(char *buffer) {
  if (CarretIndex <= 0 || TextSize <= 0)
    return;

  // 1. Сдвигаем память
  for (int i = CarretIndex - 1; i < TextSize - 1; i++) {
    buffer[i] = buffer[i + 1];
  }
  TextSize--;
  buffer[TextSize] = '\0'; // Ставим ноль в конце. БОЛЬШЕ НИКАКИХ IndexDeleteC!

  // 2. Вычисляем, откуда empezar рисовать
  int drawX = CursorPosCol - 1;
  int drawY = CursorPosRow;

  // 3. Защита от ухода в минус (если мы в начале строки)
  if (drawX < 0) {
    drawX = NUM_COLUMS - 1; // Переходим в самый конец предыдущей строки
    drawY--;
  }

  // 4. Рисуем сдвинутый текст
  for (int i = CarretIndex - 1; i < TextSize; i++) {
    PutChar(drawX, drawY, buffer[i]);
    drawX++;

    // Обязательно переносим строку при рисовании, если текст длинный!
    if (drawX >= NUM_COLUMS) {
      drawX = 0;
      drawY++;
    }
  }

  // 5. Затираем последний символ пробелом
  PutChar(drawX, drawY, ' ');

  // ВАЖНО: Мы НЕ трогаем CarretIndex и CursorPosCol здесь!
}

// Главная функция удаления
void BackSpaceHandle(char *string) {
  // Убрали странные проверки lineralpos. Если CarretIndex > 0, значит удалять
  // можно.
  if (CarretIndex <= 0 || TextSize <= 0)
    return;

  // 1. Обработка переноса курсора НАЗАД
  if (CursorPosCol == 0) {
    CursorPosRow--;            // Поднимаемся на строку вверх
    CursorPosCol = NUM_COLUMS; // Становимся в самый правый край
  }

  // 2. Рисуем и меняем буфер (передаем текущие координаты, она сама отнимет 1)
  ShiftLeft(string);

  // 3. Обновляем ВСЕ переменные состояния строго после сдвига
  CarretIndex--;  // Двигаем логический индекс
  CursorPosCol--; // Двигаем визуальную колонку

  // 4. Двигаем аппаратный курсор консоли туда, где он должен быть
  CursorPos(CursorPosCol, CursorPosRow);
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

void ShiftRight(char *buffer) {
  if (CarretIndex >= TextSize)
    return;

  // я то мыслю правильно и пишу код в правильном направлении но так впадлу
  // додумавать всю эту хунйю с индексами сторонами и тд по этому нейронка в
  // этом случаех
  // int base_offset = NUM_COLUMS * CursorPosRow;
  //
  // // ВЫЧИСЛЯЕМ СМЕЩЕНИЕ:
  // // Физическая позиция курсора минус логическая позиция в строке.
  // // Это даст нам колонку, с которой реально начинается текст на экране.
  // int visual_offset = CursorPosCol - CarretIndex;
  //
  // // Идем с КОНЦА текста в НАЧАЛО.
  // for (int i = TextSize - 1; i >= CarretIndex; i--) {
  //   // Читаем символ, учитывая РЕАЛЬНОЕ место на экране (base + смещение +
  //   // индекс)
  //   char current = buffer[base_offset + visual_offset + i];
  //
  //   // +1 сдвиг в право
  //   // buffer[base_offset + visual_offset + i + 1] = current;
  //   PixelGrapchics.DrawCharOf(base_offset + visual_offset + i + 1, current,
  //                             CONSOLE_COLOR_BLUE, CONSOLE_COLOR_BLACK);
  // }
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
      BackSpaceHandle(string);
      // CarretIndex--;
      Current_Column =
          CursorPosCol; // из за того что я еблан и у меня в консоли 2 системы
                        // координат и после того как я мувнулся влево и
                        // удалил до конца координаты шлют меня нахуй и печать
                        // начинается с того момента когда я начал удалять а
                        // символ которрый остался в начале копируется. И
                        // опять без нейронки не обошлось, но в конце концов я
                        // оставил свой вариант
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
    ShiftRight(string);
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

  // for (u8 i = 0; i < End; i++)
  // ReadedBuffer[i] = buffer[(Start + i) + MaxColumn * Line];
}
