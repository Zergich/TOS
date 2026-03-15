#include <console.h>
#include <print.h>
#include <shell.h>
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
