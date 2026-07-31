#include "System/MemoryManager/kmalloc/kmalloc.h"
#include <ConsoleIO/console.h>
#include <ConsoleIO/font.h>
#include <ConsoleIO/graphics.h>
#include <ConsoleIO/print.h>
#include <ConsoleIO/shell.h>
#include <Drivers/keyboard.h>
#include <System/sysinfo.h>
#include <libs/Array.h>
#include <libs/datastruct.h>
#include <libs/string.h>
#include <stdint.h>
#include <types.h>

extern RoundBufferObgect RoundBuff;
extern StringStruct string;

extern u16 CursorPosCol;
extern u16 CursorPosRow;

typedef struct ShellHistoryCommnad { // надо потом как нибудб решить вопрос со
                                     // статической длинной
  char NameComand[32];
  char *FullCommand;
  bool Status;
  struct ShellHistoryCommnad *Next;
} ShellHistory;

ShellHistory *global_commands_head;
ShellHistory *FindeAutoEnd(char *UserInput);

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
void CursorClear() {

  char c = ' ';
  if (cursor_last_pos.CarretIndex < TextSize) {
    c = ActiveInputBuffer[cursor_last_pos.CarretIndex];
  }

  PutChar(cursor_last_pos.Column, cursor_last_pos.Row, c);
}
void ResetCursorBlink() {
  CursorVisible = true;
  CursorBlinkTicks = 0;
  DrawConsoleCursor();
}
u16 ShellStartRow = 0; // эта херня спасает от лесенки

void Syntax(char *str);
void ClearAutoEndTail(char *str);

void ApplyAutoEnd(char *Getstring) {
  ShellHistory *his = FindeAutoEnd(Getstring);
  if (his == NULL)
    return;

  u16 user_len = string.Strlen(Getstring);
  u16 hist_len = string.Strlen(his->NameComand);

  for (u32 i = user_len; i < hist_len; i++) {
    if (TextSize >= max_len - 1)
      break;
    char c = his->NameComand[i];
    // IndexInsertC(string, &i, max_len, CarretIndex++, c);
    IndexInsertC(Getstring, &TextSize, max_len, CarretIndex++, c);
  }

  Getstring[TextSize] = '\0';
  Syntax(Getstring);

  // Принудительно заставляем шелл пересчитать экранные координаты курсора
  // на основе нашего нового, честного CarretIndex (чтобы он не улетал на +5
  // символов)
  u16 total_pos = CarretIndex + LimitXRow;
  CursorPosCol = total_pos % NUM_COLUMS;
  CursorPosRow = ShellStartRow + (total_pos / NUM_COLUMS);
}
void ArrowHandleRL(u8 ArrowType, char *string) {
  u16 lineralpos = LimitXRow + CarretIndex;
  u16 next_carret = CarretIndex;

  if (ArrowType == LeftArrow) {
    if (lineralpos > LimitXRow)
      next_carret--;
    else
      return;
  } else {
    // ЕСЛИ НАЖАЛИ ВПРАВО В САМОМ КОНЦЕ СТРОКИ -> применяем автодополнение
    if (CarretIndex >= TextSize) {
      ApplyAutoEnd(string);
      return;
    }

    if (lineralpos < TextSize + LimitXRow)
      next_carret++;
    else
      return;
  }

  CursorClear();

  // Обновление индекса
  CarretIndex = next_carret;

  // Математический расчет координат
  u16 total_pos = CarretIndex + LimitXRow;

  CursorPosCol = total_pos % NUM_COLUMS;
  CursorPosRow = ShellStartRow + (total_pos / NUM_COLUMS);
}

bool SpecCodeConsoleRead = false;
bool CheckSpecKeys(u8 SpecKey, char *string) {
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
    ArrowHandleRL(SpecKey, string);
    break;
  case DownArrow:
    print("фва фвфв фывывфыв");
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

void GetFirstCommandStatic(char *str, char *buff) {
  int cmd_len = 0;
  while (str[cmd_len] != ' ' && str[cmd_len] != '\0' && cmd_len < 31) {
    buff[cmd_len] = str[cmd_len];
    cmd_len++;
  }
  buff[cmd_len] = '\0';
}
bool IsValideCommand(char *str) {
  int result = ParseCommnad(str);
  // кароче ShellStartRow спасает от лесенки тоесть захардкоженное число 4
  // спасает от лесенки на 40 50 строках (был как приммер) и ведь жестко
  // прописанно что я перемещаюсь на 7 колонку 4 строки но нет происходит ровно
  // то что я хочу все начинается с начало послденей строки и вывод правильный и
  // красивый. Я В АХУЕ.
  // ConsoleSetCarretPos(LimitXRow, ShellStartRow);
  if (result == -1) {
    return false;
    // printf("%F%s%F",CONSOLE_COLOR_RED,str,CONSOLE_COLOR_CYAN);
  } else {
    return true;
    // printf("%F%s%F",CONSOLE_COLOR_GREEN,str,CONSOLE_COLOR_CYAN);
  }
}
void Syntax(char *str) {
  char cmd_buf[32];
  GetFirstCommandStatic(str, cmd_buf);

  bool is_valid = IsValideCommand(cmd_buf);

  bool in_quotes = false;
  bool is_command = true;

  // ЛОКАЛЬНЫЕ координаты отрисовки (не ломают твой реальный курсор!)
  u16 render_col = LimitXRow;
  u16 render_row = ShellStartRow;

  for (int i = 0; str[i] != '\0'; i++) {
    u32 color = CONSOLE_COLOR_CYAN; // По умолчанию белый

    // 1. Определение цвета
    if (str[i] == '\"') {
      in_quotes = !in_quotes; // Если была true — станет false (и наоборот)
      color = CONSOLE_COLOR_YELLOW;
    }
    // 2. Если это НЕ кавычка, но мы внутри кавычек — красим в желтый
    else if (in_quotes) {
      color = 0x07D9A6;

    } else if (str[i] == '-') {
      if (i == 0 || str[i - 1] == ' ')
        color = CONSOLE_COLOR_MAGENTA; // Мангета
    } else if (is_command) {
      color = is_valid ? CONSOLE_COLOR_GREEN : CONSOLE_COLOR_RED;
    }

    if (str[i] == ' ')
      is_command = false;
    // подсказку что это такое смотри в printf
    // Если ты обрабатываешь символ напрямую из буфера клавиатуры:
    u32 code =
        (u32)str[i]; // Это уже готовый u32 код (например, 0x0444 или 'a')
    u32 Symbol = FindGlyphIndex(code); // Переводим код в индекс глифа шрифта
    DrawChar(render_col, render_row, Symbol, color, CONSOLE_COLOR_BLACK);

    // 3. Сдвигаем локальный курсор отрисовки
    render_col++;
    if (render_col >= NUM_COLUMS) {
      render_col = 0;
      render_row++;
    }
  }
}

void RegisterAutoEnd(char *GetCommnad) {
  ShellHistory *new_node = (ShellHistory *)kmalloc(sizeof(ShellHistory));
  if (new_node == NULL)
    return;

  new_node->FullCommand = GetCommnad;
  GetFirstCommandStatic(GetCommnad, new_node->NameComand);
  new_node->Status = IsValideCommand(new_node->NameComand);
  new_node->Next = global_commands_head;
  global_commands_head = new_node;
}
ShellHistory *FindeAutoEnd(char *UserInput) {
  u16 Len = string.Strlen(UserInput);
  if (Len == 0)
    return NULL;

  ShellHistory *current = global_commands_head;
  char cmd_buf[32];

  while (current != NULL) {
    u16 name_len = string.Strlen(current->NameComand);

    if (name_len >= Len) {
      bool match = true;
      for (u16 i = 0; i < Len; i++) {
        char hist_char = current->NameComand[i];
        char user_char = UserInput[i];
        if (hist_char != user_char) {
          match = false;
          break;
        }
      }
      if (match) {
        return current;
      }
    }

    current = current->Next;
  }

  return NULL;
}
void ClearAutoEndTail(char *str) {
  u16 user_len = string.Strlen(str);
  u16 render_col = LimitXRow + user_len;
  u16 render_row = ShellStartRow;

  while (render_col >= NUM_COLUMS) {
    render_col -= NUM_COLUMS;
    render_row++;
  }

  // Просто рисуем 32 черных пробела поверх старого хвоста подсказки
  for (int i = 0; i < 32; i++) {
    if (render_row >= NUM_ROWS)
      break;

    DrawChar(render_col, render_row, ' ', CONSOLE_COLOR_BLACK,
             CONSOLE_COLOR_BLACK);

    render_col++;
    if (render_col >= NUM_COLUMS) {
      render_col = 0;
      render_row++;
    }
  }
}
void PrintAutoEnd(ShellHistory *his, char *str) {
  if (his == NULL || str == NULL)
    return;

  u16 user_len = string.Strlen(str);

  // Вычисляем, где заканчивается текст пользователя на экране.
  // Подсказка начнется ровно ТАМ, где сейчас стоит мигающий курсор.
  u16 render_col = LimitXRow + user_len;
  u16 render_row = ShellStartRow;

  while (render_col >= NUM_COLUMS) {
    render_col -= NUM_COLUMS;
    render_row++;
  }

  for (int i = user_len; his->NameComand[i] != '\0'; i++) {
    if (render_row >= NUM_ROWS)
      break;

    DrawChar(render_col, render_row, his->NameComand[i],
             CONSOLE_COLOR_LIGHT_GRAY, CONSOLE_COLOR_BLACK);

    render_col++;
    if (render_col >= NUM_COLUMS) {
      render_col = 0;
      render_row++;
    }
  }
}

int ConsoleRead(char *string) { // мб спипать спец коды и отсавлять только
                                // аски соответственно.
  // это понадобится для чтении клавиши, ведь при текущей
  // реализации читает только ascii без спец кодовх
  u32 i = 0;
  u32 c = 0;
  ActiveInputBuffer = string;
  ResetCursorBlink();
  ShellStartRow = CursorPosRow;
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
      Syntax(string);

      // из за того что я еблан и у меня в консоли 2 системы
      // координат и после того как я мувнулся влево и
      // удалил до конца координаты шлют меня нахуй и печать
      // начинается с того момента когда я начал удалять а
      // символ которрый остался в начале копируется. И
      // опять без нейронки не обошлось, но в конце концов я
      // оставил свой вариант
      CursorClear();
      i--;
      ResetCursorBlink(); // <--- сброс мигания при печати

      continue;
    }

    // Клавиша Enter обычно посылает код 0x0D ('\r'), иногда 0x0A ('\n')
    if (c == '\r' || c == '\n') { // промежуточное решение следа курсора
      CursorClear();
      if (i != 0)
        RegisterAutoEnd(string); // регистрация команд для истории
      ClearAutoEndTail(string);
      PrintChar('\n');

      CarretIndex = 0;
      TextSize = 0;
      ShellStartRow = CursorPosRow;
      ConsoleResetColor(); // сбросить цвет после подстветки синтаксиса
      // ResetCursorBlink();

      break;
    }
    if (SpecCodeConsoleRead) {
      CheckSpecKeys(c, string);
      ResetCursorBlink(); // <--- сброс мигания при печати
      SpecCodeConsoleRead = false;
      i = TextSize; // пиздец вот эта херня сэйвит все для автокомплитов
                    // синхронизация переменных без нее там такой пиздец
                    // получается аж страшно становится glm 5.2 соло

      continue;
    }
    if (c == Code_MagickCode) {
      SpecCodeConsoleRead = true;
      continue;
    }
    TextSize++;
    IndexInsertC(string, &i, max_len, CarretIndex++, c);
    ShiftRight(string);
    // PrintChar(c);
    Syntax(string); // вот эта зерня печатает текст
    ClearAutoEndTail(string);
    ShellHistory *current = FindeAutoEnd(string);
    if (current != NULL) {
      PrintAutoEnd(current, string);
    }
    // корды для курсора чтоб не убегал в конец строки потому что PrintChar
    // теперь закоменчена
    u16 total_pos = CarretIndex + LimitXRow;
    CursorPosCol = total_pos % NUM_COLUMS;
    CursorPosRow = ShellStartRow + (total_pos / NUM_COLUMS);

    ResetCursorBlink(); // <--- сброс мигания при печати
  }
  // IndexInsertC(string, &TextSize, max_len, i, '\0');

  string[i] = '\0'; // для корректного завершения строки
  return 0;
}

char ReadKey() {
  u32 GetChar;
  RoundBuff.get(&GetChar);
  if (GetChar) {
    return GetChar;
  }
  return -1;
}
