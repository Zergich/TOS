#include <types.h>
#include <System/sysinfo.h>
#include <ConsoleIO/print.h>
#include <libs/time.h>


u8 LeftPadding = 50;

const char *vga_logo[] = {
    "                            XXXXXXX\n",
    "                           XXX      XX\n",
    "                              XXX      XX\n",
    "                           XXXXX         XX\n",
    "                        XXX        X      XX\n",
    "               XXXX  XXX        XXXXXXX    XXXX\n",
    "               X   XXXX      XXXXXXXXXXXXXX   X\n",
    "               X      XXX  XXXXXXXXXXXXXX      X\n",
    "               X         XXXXXXXXXXXXX        XX\n",
    "               X    XX      XXXXXXX      XXXXXXX\n",
    "               X    XXXXX      XX     XXXXX    X\n",
    "               X    XXXXXXX    X    XXXX       X\n",
    "               X    XXXXXXX    X               X\n",
    "               X    XXXXXXX    X       XXXX    X\n",
    "               X     XXXXXX    X    XXXXXX     X\n",
    "               XXX      XXX    XXXXXXX      XXX\n",
    "                  XXX          XX        XX\n",
    "                    XXXXX      X      XX\n",
    "                        XXXX   X   XX\n",
    "                         XXXXXXXXXX\n",
};
void PrintInfo()
{
  ConsoleSetCarretPos(LeftPadding,CursorPosRow+5);
  printf("%FOS%F: TOS %s\n",CONSOLE_COLOR_GREEN,CONSOLE_COLOR_WHITE,VersionOS);
  CursorSetColumn(LeftPadding);
  printf("%FUptime%F: %u minute\n",CONSOLE_COLOR_GREEN,CONSOLE_COLOR_WHITE,Timepit.PitTimerSecondsUp/1000);
  CursorSetColumn(LeftPadding);
  struct DateTime now = GetTimeRTS();
  printf("%FTime%F: %u-%u-%u %u:%u:%u\n",CONSOLE_COLOR_GREEN,CONSOLE_COLOR_WHITE,now.day, now.month, now.year, now.hour,
           now.minute, now.second);

  CursorSetColumn(LeftPadding);
  printf("%FShell%F: Cuper %s\n",CONSOLE_COLOR_GREEN,CONSOLE_COLOR_WHITE,ShellVersion);
  CursorSetColumn(LeftPadding);
printf("%FCPU%F: %s \n", 
       CONSOLE_COLOR_GREEN, // Цвет префикса "CPU:"
       CONSOLE_COLOR_WHITE, // Возврат к обычному цвету
       CPUInfo.brand); 
  CursorSetColumn(LeftPadding);
  printf("%FScreen resolution%F: %ux%u\n", CONSOLE_COLOR_GREEN,CONSOLE_COLOR_WHITE,  WidthDisplay,
         HeightDisplay);
    CursorSetColumn(LeftPadding);

  printf("%FConsole resolution%F: %ux%u\n", CONSOLE_COLOR_GREEN,CONSOLE_COLOR_WHITE,  NUM_COLUMS,
         NUM_ROWS);

  CursorSetColumn(LeftPadding);
  struct MemoryType MemInfo =  ReturnMemoryMap();
  printf("%FMemory%F: Usable %uMiB / Reserved %uMiB / Total %uMiB\n",CONSOLE_COLOR_GREEN,CONSOLE_COLOR_WHITE,MemInfo.UsableMemory,MemInfo.ReservedMemory,MemInfo.UsableMemory+MemInfo.ReservedMemory);
  
  ConsoleForeground(CONSOLE_COLOR_CYAN);
}

void DrawCubeLogo(u16 pos,u16 row) {
int rows = sizeof(vga_logo) / sizeof(vga_logo[0]);
  ConsoleSetCarretPos(pos,row);
    for (int i = 0; i < rows; i++) {
        // Выводим i-ю строку, увеличивая координату Y на i
                printf("%F%s%F",0x5027F5,vga_logo[i], CONSOLE_COLOR_CYAN);
    }

}

void Fetch()
{
  u16 pos = CursorPosCol;
  u16 row = CursorPosRow;
PrintInfo();
DrawCubeLogo(pos,row);
}


