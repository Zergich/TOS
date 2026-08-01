#include <ConsoleIO/print.h>
#include <System/MemoryManager/kmalloc/kmalloc.h>
#include <System/sysinfo.h>
#include <libs/format.h>
#include <libs/time.h>
#include <types.h>

u8 LeftPadding = 50;

const u32 *vga_logo[] = {
    U"                            XXXXXXX\n",
    U"                           XXX      XX\n",
    U"                              XXX      XX\n",
    U"                           XXXXX         XX\n",
    U"                        XXX        X      XX\n",
    U"               XXXX  XXX        XXXXXXX    XXXX\n",
    U"               X   XXXX      XXXXXXXXXXXXXX   X\n",
    U"               X      XXX  XXXXXXXXXXXXXX      X\n",
    U"               X         XXXXXXXXXXXXX        XX\n",
    U"               X    XX      XXXXXXX      XXXXXXX\n",
    U"               X    XXXXX      XX     XXXXX    X\n",
    U"               X    XXXXXXX    X    XXXX       X\n",
    U"               X    XXXXXXX    X               X\n",
    U"               X    XXXXXXX    X       XXXX    X\n",
    U"               X     XXXXXX    X    XXXXXX     X\n",
    U"               XXX      XXX    XXXXXXX      XXX\n",
    U"                  XXX          XX        XX\n",
    U"                    XXXXX      X      XX\n",
    U"                        XXXX   X   XX\n",
    U"                         XXXXXXXXXX\n",
};
void PrintInfo() {
  ConsoleSetCarretPos(LeftPadding, CursorPosRow + 5);
  printf("%FOS%F: TOS %s\n", CONSOLE_COLOR_GREEN, CONSOLE_COLOR_WHITE,
         VersionOS);
  CursorSetColumn(LeftPadding);
  printf("%FUptime%F: %u minute\n", CONSOLE_COLOR_GREEN, CONSOLE_COLOR_WHITE,
         Timepit.PitTimerSecondsUp / 1000);
  CursorSetColumn(LeftPadding);
  struct DateTime now = GetTimeRTS();
  printf("%FTime%F: %u-%u-%u %u:%u:%u\n", CONSOLE_COLOR_GREEN,
         CONSOLE_COLOR_WHITE, now.day, now.month, now.year, now.hour,
         now.minute, now.second);

  CursorSetColumn(LeftPadding);
  printf("%FShell%F: Cuper %s\n", CONSOLE_COLOR_GREEN, CONSOLE_COLOR_WHITE,
         ShellVersion);
  CursorSetColumn(LeftPadding);
  printf("%FCPU%F: %s \n",
         CONSOLE_COLOR_GREEN, // Цвет префикса "CPU:"
         CONSOLE_COLOR_WHITE, // Возврат к обычному цвету
         CPUInfo.brand);
  CursorSetColumn(LeftPadding);
  printf("%FScreen resolution%F: %ux%u\n", CONSOLE_COLOR_GREEN,
         CONSOLE_COLOR_WHITE, WidthDisplay, HeightDisplay);
  CursorSetColumn(LeftPadding);

  printf("%FConsole resolution%F: %ux%u\n", CONSOLE_COLOR_GREEN,
         CONSOLE_COLOR_WHITE, NUM_COLUMS, NUM_ROWS);

  CursorSetColumn(LeftPadding);
  struct MemoryType MemInfo = ReturnMemoryMap();
  printf("%FMemory%F: Usable %uMiB / Reserved %uMiB / Total %uMiB\n",
         CONSOLE_COLOR_GREEN, CONSOLE_COLOR_WHITE, MemInfo.UsableMemory,
         MemInfo.ReservedMemory, MemInfo.UsableMemory + MemInfo.ReservedMemory);

  tlsf_stats_t MemHeapStatusInfo = MemHeapInfo();
  CursorSetColumn(LeftPadding);
  printf("%FMem Usage (K_Heap)%F: ", CONSOLE_COLOR_GREEN, CONSOLE_COLOR_WHITE);
  print_memory_size(MemHeapStatusInfo.free_bytes);
  print(" / ");
  print_memory_size(MemHeapStatusInfo.used_bytes);
  print("\n");
  ConsoleForeground(CONSOLE_COLOR_CYAN);
}

void DrawCubeLogo(u16 pos, u16 row) {
  int rows = sizeof(vga_logo) / sizeof(vga_logo[0]);
  ConsoleSetCarretPos(pos, row);
  for (int i = 0; i < rows; i++) {
    // Выводим i-ю строку, увеличивая координату Y на i
    printf("%F%S%F", 0x5027F5, vga_logo[i], CONSOLE_COLOR_CYAN);
  }
}

void Fetch() {
  u16 pos = CursorPosCol;
  u16 row = CursorPosRow;
  PrintInfo();
  DrawCubeLogo(pos, row);
}
