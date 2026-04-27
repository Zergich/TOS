#include <ConsoleIO/console.h>
#include <ConsoleIO/print.h>
#include <ConsoleIO/shell.h>
#include <System/sysinfo.h>
#include <string.h>
#include <types.h>
extern ConsoleInput Console;
extern StringStruct string;

enum ShellCommand {
  NotACommand = -1,
  Pede = 0,
  Mem = 1,
  WhiteSpace = 2,
  Echo = 3,
};

bool EmptyString = false;
int ParseCommnad(
    char *str) { // очевидная проблема в том что пока эта херь не сплитает
                 // строки и команды с флагами не сработают
  // закомментированные функции нужны потому что мб через if сделаю
  if (string.IsEmptyOrWhitespace(str)) {
    EmptyString = true;
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
  return NotACommand;
}
void ShellCommandEnding() {
  if (!EmptyString) {
    print("\n");
    EmptyString = false;
  }
  printf("Shell> ");
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
    PrintMem();
    break;
  }
  // print(pede);
  ShellCommandEnding();
}
