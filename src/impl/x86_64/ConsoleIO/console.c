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

void EraseCursorTrail() {
  if (ActiveInputBuffer == 0)
    return;

  char char_to_restore = ' ';

  // Берем символ, который находится ПОД курсором прямо сейчас
  if (CarretIndex < TextSize) {
    char_to_restore = ActiveInputBuffer[CarretIndex];
  }

  u64 px = CursorPosCol * FONT_WIDTH;
  u64 py = CursorPosRow * FONT_HEIGHT;

  // ПЕРЕЗАПИСЫВАЕМ старую позицию курсора обычными цветами (без белого
  // квадрата)
  PutChar(CursorPosCol, CursorPosRow, char_to_restore);
}
void ShiftLeft(char *buffer) {
  if (CarretIndex <= 0 || TextSize <= 0)
    return;

  // 1. Сдвигаем память
  for (int i = CarretIndex - 1; i < TextSize - 1; i++) {
    buffer[i] = buffer[i + 1];
  }
  TextSize--;
  buffer[TextSize] = '\0';
  // 2. Вычисляем корды
  int drawX = CursorPosCol - 1;
  int drawY = CursorPosRow;

  // 3. Защита от ухода в минус
  if (drawX < 0) {
    drawX = NUM_COLUMS - 1; // Переходим в самый конец предыдущей строки
    drawY--;
  }

  // 4. Перерисовка текста тот самый сдвиг
  for (int i = CarretIndex - 1; i < TextSize; i++) {
    PutChar(drawX, drawY, buffer[i]);
    drawX++;

    //  перенос текста на новую строку
    if (drawX >= NUM_COLUMS) {
      drawX = 0;
      drawY++;
    }
  }

  // 5. Затираем последний символ пробелом
  PutChar(drawX, drawY, ' ');
  EraseCursorTrail();
}

void BackSpaceHandle(char *string) {
  if (CarretIndex <= 0 || TextSize <= 0)
    return;

  // 1. Обработка переноса курсора НАЗАД
  if (CursorPosCol == 0) {
    CursorPosRow--;            // Поднимаемся на строку вверх
    CursorPosCol = NUM_COLUMS; // Становимся в самый правый край
  }

  // 2. Рисуем и меняем буфер
  ShiftLeft(string);

  CarretIndex--;  // Двигаем логический индекс
  CursorPosCol--; // Двигаем визуальную колонку

  // 4. Двигаем аппаратный курсор консоли туда, где он должен быть
  EraseCursorTrail();
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
  // Защита: если мы печатаем в самый конец строки,
  // визуально сдвигать нечего (CursorPosCol уже там, где нужно).
  // CarretIndex здесь уже увеличен на 1 в ConsoleRead, поэтому сравниваем с
  // TextSize.
  if (CarretIndex >= TextSize)
    return;

  int drawX = CursorPosCol;
  int drawY = CursorPosRow;

  // Так как ConsoleRead сделал CarretIndex++ ДО вызова этой функции,
  // индекс только что вставленного символа равен (CarretIndex - 1).
  // Рисуем от него до конца строки.
  for (int i = CarretIndex - 1; i < TextSize; i++) {

    // Защита от мусора на всякий случай
    if (buffer[i] == '\0')
      break;

    PutChar(drawX, drawY, buffer[i]);
    drawX++;

    // Переход на новую строк

    if (drawX >= NUM_COLUMS) {
      drawX = 0;
      drawY++;

      // Защита от ухода за экран (чтобы не было бесконечности)
      if (drawY >= NUM_ROWS) { // Замените MAX_ROWS на вашу высоту
        break;
      }
    }
  }

  // Затираем крайний символ пробелом, так как текст стал на 1 символ длиннее
  PutChar(drawX, drawY, ' ');
}

// 1 - курсор видим, 0 - скрыт
u8 CursorVisible = 1;

// Счетчик тиков таймера для управления скоростью моргания
u32 CursorBlinkTicks = 0;

// Скорость моргания
u16 CURSOR_BLINK_RATE = 200;

char *ActiveInputBuffer = 0;

void DrawConsoleCursor() {
  if (ActiveInputBuffer == 0)
    return;

  char char_to_draw = ' '; // Символ по умолчанию (если курсор в самом конце)

  if (CarretIndex < TextSize) {
    char_to_draw =
        ActiveInputBuffer[CarretIndex]; // Берем РЕАЛЬНЫЙ символ из буфера
  }

  if (CursorVisible) {
    u64 px = CursorPosCol * FONT_WIDTH;
    u64 py = CursorPosRow * FONT_HEIGHT;
    PixelGrapchics.DrawChar(px, py, char_to_draw, CONSOLE_COLOR_BLACK,
                            CONSOLE_COLOR_LIGHT_GRAY);

  } else {
    // Курсор невидим -> просто рисуем то, что должно быть на экране
    PutChar(CursorPosCol, CursorPosRow, char_to_draw);
  }
}
void ResetCursorBlink() {
  CursorVisible = 1;    // Делаем курсор сразу видимым
  CursorBlinkTicks = 0; // Сбрасываем таймер
  DrawConsoleCursor();  // Рисуем
}

int ConsoleRead(char *string) { // мб спипать спец коды и отсавлять только
                                // аски соответственно.
  // это понадобится для чтении клавиши, ведь при текущей
  // реализации читает только ascii без спец кодовх
  u32 i = 0;
  u8 c = 0;
  ActiveInputBuffer = string;

  ResetCursorBlink();
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
      ResetCursorBlink(); // <--- сброс мигания при удалении
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
      ResetCursorBlink(); // <--- сброс мигания при печати
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
    ResetCursorBlink(); // <--- сброс мигания при печати
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
