#include <ConsoleIO/print.h>
#include <System/Mion/GeometryWindow.h>
#include <System/sysinfo.h>
#include <libs/string.h>
#include <types.h>

enum LinesConfig {
  RightCorner = '/',
  LeftCorner = '\\',

  LineVertical = '|',
  LineHorisontal = '-'
};

// Символы псевдографики для рамки (Unicode / ASCII коды)
#define CHAR_TOP_LEFT '\xC9'     // ╔
#define CHAR_TOP_RIGHT '\xBB'    // ╗
#define CHAR_BOTTOM_LEFT '\xC8'  // ╚
#define CHAR_BOTTOM_RIGHT '\xBC' // ╝
#define CHAR_HORIZONTAL '\xCD'   // ═
#define CHAR_VERTICAL '\xBA'     // ║

// Функция для отрисовки прямоугольника
void DrawWindow(struct WindowInfo *win) {
  if (win->Color == 0) // NULL
    win->Color = HexColor("#1F1FA3");

  ConsoleBackground(win->Color);
  // Защита от выхода за пределы экрана (чтобы ядро не крашнулось)
  if (win->PosX >= NUM_COLUMS || win->PosY >= NUM_ROWS)
    return;
  if (win->Width < 5 || win->Height < 3)
    return; // Минимальный размер окна 2x2

  // Вычисляем границы, чтобы не выходить за пределы экрана
  u16 max_x = win->PosX + win->Width - 1;
  u16 max_y = win->PosY + win->Height - 1;

  if (max_x >= NUM_COLUMS)
    max_x = NUM_COLUMS - 1;
  if (max_y >= NUM_ROWS)
    max_y = NUM_ROWS - 1;

  PutChar(win->PosX, win->PosY, CHAR_TOP_LEFT); // Левый верхний угол
  for (u16 x = win->PosX + 1; x < max_x; x++) {
    PutChar(x, win->PosY, CHAR_HORIZONTAL); // Горизонтальная линия
  }
  PutChar(max_x, win->PosY, CHAR_TOP_RIGHT); // Правый верхний угол

  for (u16 y = win->PosY + 1; y < max_y; y++) {
    PutChar(win->PosX, y, CHAR_VERTICAL); // Левая вертикальная линия

    // Заполнение внутренности окна пробелами
    for (u16 x = win->PosX + 1; x < max_x; x++) {
      PutChar(x, y, ' ');
    }

    PutChar(max_x, y, CHAR_VERTICAL); // Правая вертикальная линия
  }

  PutChar(win->PosX, max_y, CHAR_BOTTOM_LEFT); // Левый нижний угол
  for (u16 x = win->PosX + 1; x < max_x; x++) {
    PutChar(x, max_y, CHAR_HORIZONTAL); // Горизонтальная линия
  }
  PutChar(max_x, max_y, CHAR_BOTTOM_RIGHT); // Правый нижний угол
  SetTitle(win);
  SetButtonsMenu(win);
}

void SetTitle(struct WindowInfo *win) {
  if (win->Width < 10)
    return;
  ConsoleSetCarretPos(win->PosX + 1, win->PosY);

  for (int i = 0; i < string.Strlen(win->Title); i++) {
    if (i < win->Width - 10) // 5 символо в на кнопки 3 на троеточие и еще 2 на
                             // показ линии pede...== - X
      print(win->Title[i]);
    else {
      print("...");
      break;
    }
  }
}
void SetButtonsMenu(struct WindowInfo *win) {
  ConsoleSetCarretPos(win->PosX + win->Width - 5, win->PosY);
  print(" - X");
}
