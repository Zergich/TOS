#include <OSInit.h>
#include <print.h>
#include <stdint.h>
#include <sysinfo.h>

char *VersionOS = "0.0.1";

void WelcomeMessage() {
  ConsoleSetCursorPos(NUM_COLUMS / 3, 0); // центрирование
  ConsoleColor(CONSOLE_COLOR_GREEN, CONSOLE_COLOR_BLACK);
  print("Welcome to TOS!\n");
  ConsoleSetCursorPos(NUM_COLUMS / 3, 1);
  printf("OS Version: %s\n", VersionOS);
  ApperLine();
}

void ApperLine() {
  ConsoleColor(CONSOLE_COLOR_LIGHT_GREEN, CONSOLE_COLOR_BLACK);
  for (uint8_t i = 0; i < NUM_COLUMS; i++)
    print("-");
  print("\n");
  ConsoleResetColor();
  ConsoleForeground(CONSOLE_COLOR_CYAN);
  print("Shell> ");
}
