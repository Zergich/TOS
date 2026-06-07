#include "types.h"
#include <ConsoleIO/font.h>
#include <ConsoleIO/print.h>
#include <System/OSInit.h>
#include <System/sysinfo.h>
#include <VGA/vgacursor.h>

// Инициализация констант

char *VersionOS = "ALPHA";

u64 BufferPitchW = 0;
u16 HeightDisplay = 0;
u16 WidthDisplay = 0;

u16 NUM_COLUMS;
u16 NUM_ROWS;

void Enabled();
void WelcomeMessage() {
  ConsoleClear();
  ConsoleSetCarretPos(NUM_COLUMS / 3, 0); // центрирование
  ConsoleColor(CONSOLE_COLOR_GREEN, CONSOLE_COLOR_BLACK);
  print("Welcome to TOS!\n");
  ConsoleSetCarretPos(NUM_COLUMS / 3, 1);
  printf("OS Version: %s\n", VersionOS);
  ApperLine();

  Enabled();
}

// надо потому что иначе начальное значние убивается и после первого символа
// ввода с клавы курсор улетает на хуйх
extern u16 CursorPosCol;
extern u16 CursorPosRow;

void ApperLine() {
  ConsoleColor(CONSOLE_COLOR_LIGHT_GREEN, CONSOLE_COLOR_BLACK);
  for (u8 i = 0; i < NUM_COLUMS; i++)
    print("-");
  print("\n");
  ConsoleResetColor();
  ConsoleForeground(CONSOLE_COLOR_CYAN);
  print("Shell> ");

  CursorPosCol = 7;
  CursorPosRow = 4;
  CursorPos(CursorPosCol, CursorPosRow);
}

// задумка пиздатая но пока не нашел ей действительно стоющего применения
void InitConstantGraphics(u64 Pitch, u16 Width, u16 Hiegth) {

  WidthDisplay = Width;
  HeightDisplay = Hiegth;
  BufferPitchW = Pitch;

  NUM_ROWS = HeightDisplay / FONT_HEIGHT;
  NUM_COLUMS = BufferPitchW / FONT_WIDTH;
}
void SystemStatus(bool command) {
  // что то устанавливает переменные в другие координаты
  // а в качесиве команды сучек устанавливает такую позицию и все плывет.х
  if (!command) {
    CursorPosCol = 0;
    CursorPosRow = 0;
  }
  ConsoleForeground(CONSOLE_COLOR_CYAN);

  printf("OS version: %F%s%F\n", CONSOLE_COLOR_GREEN, VersionOS,
         CONSOLE_COLOR_CYAN);

  printf("Screen resolution: %F%ux%u%F\n", CONSOLE_COLOR_GREEN, WidthDisplay,
         HeightDisplay, CONSOLE_COLOR_CYAN);
  printf("Console resolution: %F%ux%u%F\n", CONSOLE_COLOR_GREEN, NUM_COLUMS,
         NUM_ROWS, CONSOLE_COLOR_CYAN);

  struct MemoryType MemMap = ReturnMemoryMap();
  printf("Memory size.\nActive: %F%u%F MB.\n", CONSOLE_COLOR_GREEN,
         MemMap.UsableMemory, CONSOLE_COLOR_CYAN);
  printf("Reserved: %F%u%F MB.\n", CONSOLE_COLOR_GREEN, MemMap.ReservedMemory,
         CONSOLE_COLOR_CYAN);
  printf("Total: %F%u%F MB.\n", CONSOLE_COLOR_GREEN,
         MemMap.ReservedMemory + MemMap.UsableMemory, CONSOLE_COLOR_CYAN);
}
void Enabled() {
  // Правильное чтение и запись MSR в x86_64
  uint32_t efer_low, efer_high;

  // 1. Читаем EFER
  __asm__ volatile("rdmsr"
                   : "=a"(efer_low), "=d"(efer_high)
                   : "c"(0xC0000080)); // Адрес EFER

  // 2. Собираем 64-битное значение
  uint64_t efer = ((uint64_t)efer_high << 32) | (uint64_t)efer_low;

  // 3. Устанавливаем 11-й бит (NXE)
  efer |= (1ULL << 11);

  // 4. Разбиваем обратно на два 32-битных слова
  efer_low = (uint32_t)(efer & 0xFFFFFFFF);
  efer_high = (uint32_t)(efer >> 32);

  // 5. Записываем обратно
  __asm__ volatile("wrmsr" ::"a"(efer_low), "d"(efer_high), "c"(0xC0000080));
}
