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

/*
 * ВАЖНО!!!
 * ТУТ БУДУТ ЗАМЕТКИ
 *
 * а что если на функцию перемещения стрелок повесить херню. ту же самую функцию
 * которая отвчает за моргание но именнно затирать предыдущий курсор какой
 * гениальный бред
 *
 *
 *
 */

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
}
// 1 - курсор видим, 0 - скрыт
bool CursorVisible = true;

// Счетчик тиков таймера для управления скоростью моргания
u32 CursorBlinkTicks = 0;

// Скорость моргания
u16 CURSOR_BLINK_RATE = 250;

char *ActiveInputBuffer;

struct CursorLastPos {
  u16 Column;
  u16 Row;
  u16 CarretIndex;
};
struct CursorLastPos cursor_last_pos;
void DrawConsoleCursor() {
  if (ActiveInputBuffer == 0)
    return;

  char char_to_draw = ' '; // Символ по умолчанию (если курсор в самом конце)

  if (CarretIndex < TextSize) {
    char_to_draw =
        ActiveInputBuffer[CarretIndex]; // Берем РЕАЛЬНЫЙ символ из буфера
  }

  if (CursorVisible) {

    PixelGrapchics.DrawChar(CursorPosCol, CursorPosRow, char_to_draw,
                            CONSOLE_COLOR_BLACK, CONSOLE_COLOR_LIGHT_GRAY);

  } else {
    // Курсор невидим -> просто рисуем то, что должно быть на экране
    PutChar(CursorPosCol, CursorPosRow, char_to_draw);
  }

  cursor_last_pos.Column = CursorPosCol;
  cursor_last_pos.Row = CursorPosRow;
  cursor_last_pos.CarretIndex = CarretIndex;
}
void CursorClear(bool IsEnter) {
  if (IsEnter) { // когда я задержываю энетер он не оставлял следа но неприятно
                 // мигал а вот собственно и решение проблемы
    u64 shadow_row = cursor_last_pos.Row - 1;
    u64 shadow_col = cursor_last_pos.Column;
    PutChar(shadow_col, shadow_row, ' ');
    // ретурн не нужен без него не робии почему то
  }

  // для стирания старого курсора на старом месте
  if (cursor_last_pos.CarretIndex < TextSize) {
    char char_at_cursor = ActiveInputBuffer[cursor_last_pos.CarretIndex];
    PutChar(cursor_last_pos.Column, cursor_last_pos.Row, char_at_cursor);
  } else {
    // Если курсор стоял в самом конце текста
    PutChar(cursor_last_pos.Column, cursor_last_pos.Row, ' ');
  }
  // а здусб защита от тени при старом коде была тень + эта же неполная функция
  // давала 1 символ с выделением + куроср
  u64 shadow_index = cursor_last_pos.CarretIndex + 1;
  if (shadow_index < TextSize) {
    char shadow_char = ActiveInputBuffer[shadow_index];

    u64 shadow_col = cursor_last_pos.Column + 1;
    u64 shadow_row = cursor_last_pos.Row;

    PutChar(shadow_col, shadow_row, shadow_char);
  }
}
void ResetCursorBlink() {
  CursorVisible = true; // Делаем курсор сразу видимым
  CursorBlinkTicks = 0; // Сбрасываем таймер
  DrawConsoleCursor();  // Рисуем
}
void MoveCursorRight() {
  // Если мы еще не дошли до правого края
  if (CursorPosCol < NUM_COLUMS - 1) {
    CursorPosCol++;
  }
  // Если мы уперлись в край — переходим на новую строку
  else {
    CursorPosCol = 0;
    CursorPosRow++;
  }

  // Проверка на выход за нижнюю границу экрана
  if (CursorPosRow >= NUM_ROWS) {
    // Здесь должен быть ваш вызов ScrollScreen();
    CursorPosRow = NUM_ROWS - 1;
  }
}
u16 ShellStartRow = 4; // эта херня спасает от лесенки

void ArrowHandleRL(u8 ArrowType) // пока только право лево
{
  u16 lineralpos = LimitXRow + CarretIndex;
  u16 next_carret = CarretIndex;

  if (ArrowType == LeftArrow) {
    if (lineralpos > LimitXRow)
      next_carret--;
    else
      return;
  } else {
    if (lineralpos < TextSize + LimitXRow)
      next_carret++;
    else
      return;
  }

  CursorClear(false);

  // обновление индекс
  CarretIndex = next_carret;

  // математический расчет координат
  u16 total_pos = CarretIndex + LimitXRow;

  CursorPosCol = total_pos % NUM_COLUMS;

  CursorPosRow = ShellStartRow + (total_pos / NUM_COLUMS);
  // CursorPosRow = CursorPosRow + (total_pos / NUM_COLUMS);
  // тут была лесенка из за проблемы накопления я бы сам в жизни никогда не
  // догадался бы
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
      BackSpaceHandle(string);
      Current_Column =
          CursorPosCol; // из за того что я еблан и у меня в консоли 2 системы
                        // координат и после того как я мувнулся влево и
                        // удалил до конца координаты шлют меня нахуй и печать
                        // начинается с того момента когда я начал удалять а
                        // символ которрый остался в начале копируется. И
                        // опять без нейронки не обошлось, но в конце концов я
                        // оставил свой вариант
      CursorClear(false);

      ResetCursorBlink(); // <--- сброс мигания при печати

      continue;
    }

    // Клавиша Enter обычно посылает код 0x0D ('\r'), иногда 0x0A ('\n')
    if (c == '\r' || c == '\n') { // промежуточное решение следа курсора

      PrintChar('\n');
      CursorClear(true);

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
  ShellStartRow = CursorPosRow; // спасает от лесенки

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
