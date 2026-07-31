#include "System/OSInit.h"
#include "libs/time.h"
#include <ConsoleIO/Fetch/fetch.h>
#include <ConsoleIO/console.h>
#include <ConsoleIO/print.h>
#include <ConsoleIO/shell.h>
#include <System/MemoryManager/kmalloc/kmalloc.h>
#include <System/sysinfo.h>
#include <libs/format.h>
#include <libs/string.h>
#include <types.h>

extern ConsoleInput Console;

enum ShellCommand {
  NotACommand = -1,
  Pede = 0,
  Mem = 1,
  WhiteSpace = 2,
  Echo = 3,
  ConsoleClearCommand = 4,
  PrintStatusCommand = 5,
  UpTimeWorkSystem = 6,
  GetTime = 7,
  FetchProgram = 8,
  CPUPrintInfo = 9,
};

int ParseCommnad(u32 *str) { // очевидная проблема в том что пока эта херь не
                             // сплитает строки и команды с флагами не сработают
  // закомментированные функции нужны потому что мб через if сделаю
  if (string.IsEmptyOrWhitespace(str)) {
    return WhiteSpace;
  }
  if (string.Strcmp(str, U"pede") == 0) {
    // print("pede123");
    return Pede;
  }
  if (string.Strcmp(str, U"mem") == 0) {
    // PrintMem();
    return Mem;
  }
  if (string.Strcmp(str, U"echo") == 0) {
    // print(str);
    return Echo;
  }
  if (string.Strcmp(str, U"clear") == 0) {
    // ConsoleClear();
    return ConsoleClearCommand;
  }
  if (string.Strcmp(str, U"status") == 0) {
    // SystemStatus();
    return PrintStatusCommand;
  }
  if (string.Strcmp(str, U"uptime") == 0) {
    // Timepit.UpTime();
    return UpTimeWorkSystem;
  }
  if (string.Strcmp(str, U"time") == 0) {
    // return GetTime    struct DateTime now = GetTimeRTS();
    //
    // printf("Time: %u-%u-%u %u:%u:%u\n", now.day, now.month, now.year,
    // now.hour,now.minute, now.second);
    return GetTime;
  }
  if (string.Strcmp(str, U"cpuinfo") == 0) {
    // PintInfoCPU();
    return CPUPrintInfo;
  }
  if (string.Strcmp(str, U"fetch") == 0) {
    // Fetch();
    return FetchProgram;
  }
  return NotACommand;
}
void ShellCommandEnding() { printf("Shell> "); }

void PrintMemoryMap(struct MemoryType mem) {
  printf("Usable Memory - %u MB\n", mem.UsableMemory);
  printf("Reserved Memory - %u MB\n", mem.ReservedMemory);
  printf("Total - %u MB\n", mem.ReservedMemory + mem.UsableMemory);

  tlsf_stats_t MemHeapStatusInfo = MemHeapInfo();
  print("Mem Usage (K_Heap): ");
  print_memory_size(MemHeapStatusInfo.free_bytes);
  print(" / ");
  print_memory_size(MemHeapStatusInfo.used_bytes);
  print("\n");
  ConsoleForeground(CONSOLE_COLOR_CYAN);
}

void Shell() {
  static string15 pede;
  Console.ReadLine(pede);
  int argc = 0;
  u32 **argv = string.Split(pede, ' ', &argc);

  if (argv == NULL) // значит был ентер
  {
    ShellCommandEnding();
    return;
  }

  int result = ParseCommnad(argv[0]);
  switch (result) {
  case WhiteSpace:
    break;

  case NotACommand:
    printf("\"%F%s%F\" - Is not a command.\n", CONSOLE_COLOR_YELLOW, pede,
           CONSOLE_COLOR_CYAN);
    break;
  case Pede:
    print("pede123\n");
    break;

  case Mem:
    PrintMemoryMap(ReturnMemoryMap());
    break;
  case ConsoleClearCommand:
    ConsoleClear();
    break;
  case PrintStatusCommand:
    SystemStatus(true);
    break;
  case UpTimeWorkSystem:
    Timepit.UpTime();
    break;
  case GetTime: { // фигурные скобки нужну чтобы обозначить область видимости
                  // если начать блок case со структуры то это не очень хорошо
    struct DateTime now = GetTimeRTS();

    printf("Time: %u-%u-%u %u:%u:%u\n", now.day, now.month, now.year, now.hour,
           now.minute, now.second);
    break;
  }
  case FetchProgram:
    Fetch();
    break;
  case CPUPrintInfo:
    PintInfoCPU();
    break;
  }
  kfree(argv);
  ShellCommandEnding();
}
