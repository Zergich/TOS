#include <VGA/console.h>
#include <VGA/print.h>
#include <VGA/shell.h>
#include <string.h>
#include <types.h>

extern ConsoleInput Console;
extern StringStruct string;

void Shell() {
  static string15 pede;
  Console.ReadLine(pede);
  if (string.Strcmp(pede, "pede") == 0)
    print("pede123");
  print(pede);
  print("\n");
  printf("Shell> ");
}
