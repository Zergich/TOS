#include <VGA/console.h>
#include <VGA/print.h>
#include <VGA/shell.h>
#include <string.h>
#include <types.h>

extern ConsoleInput Console;

void Shell() {
  static string pede;
  Console.ReadLine(pede);
  if (strcmp(pede, "pede") == 0)
    print("pede123");
  print(pede);
  print("\n");
  printf("Shell> ");
}
