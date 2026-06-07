#pragma once
#include <types.h>

struct WindowInfo {
  u16 Height;
  u16 Width;

  u16 PosX;
  u16 PosY;

  char *Title;

  u32 Color;
};

void SetTitle(struct WindowInfo *win);
void SetButtonsMenu(struct WindowInfo *win);
void DrawWindow(struct WindowInfo *info);
