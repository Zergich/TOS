#include "System/OSInit.h"
#include <ConsoleIO/console.h>
#include <ConsoleIO/print.h>
#include <ConsoleIO/shell.h>
#include <System/sysinfo.h>
#include <libs/string.h>
#include <types.h>
extern ConsoleInput Console;
extern StringStruct string;

enum ShellCommand {
  NotACommand = -1,
  Pede = 0,
  Mem = 1,
  WhiteSpace = 2,
  Echo = 3,
  ConsoleClearCommand = 4,
  PrintStatusCommand = 5,
};

int ParseCommnad(
    char *str) { // очевидная проблема в том что пока эта херь не сплитает
                 // строки и команды с флагами не сработают
  // закомментированные функции нужны потому что мб через if сделаю
  if (string.IsEmptyOrWhitespace(str)) {
    return WhiteSpace;
  }
  if (string.Strcmp(str, "pede") == 0) {
    // print("pede123");
    return Pede;
  }
  if (string.Strcmp(str, "mem") == 0) {
    // PrintMem();
    return Mem;
  }
  if (string.Strcmp(str, "echo") == 0) {
    // print(str);
    return Echo;
  }
  if (string.Strcmp(str, "clear") == 0) {
    // ConsoleClear();
    return ConsoleClearCommand;
  }
  if (string.Strcmp(str, "status") == 0) {
    // SystemStatus();
    return PrintStatusCommand;
  }
  return NotACommand;
}
void ShellCommandEnding() { printf("Shell> "); }

void PrintMemoryMap(struct MemoryType mem) {
  printf("Usable Memory - %u MB\n", mem.UsableMemory);
  printf("Reserved Memory - %u MB\n", mem.ReservedMemory);
  printf("Total - %u MB", mem.ReservedMemory + mem.UsableMemory);
}

void Shell() {
  static string15 pede;
  Console.ReadLine(pede);

  int result = ParseCommnad(pede);
  switch (result) {
  case WhiteSpace:
    break;

  case NotACommand:
    printf("\"%s\" - Is not a command.\n", pede);
    break;
  case Pede:
    print("pede123");
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
  }
  // print(pede);
  ShellCommandEnding();
}
