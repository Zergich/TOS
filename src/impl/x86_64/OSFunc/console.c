#include <console.h>
#include <datastruct.h>
#include <print.h>

extern RoundBufferObgect RoundBuff;

ConsoleInput Console = {.ReadLine = ConsoleRead, .ReadKey = ReadKey

};

char *
ConsoleRead() { // мб спипать спец коды и отсавлять только аски соответственно.
                // это понадобится для чтении клавиши, ведь при текущей
                // реализации читает только ascii без спец кодовх

  // while(RoundBuff.get())

  return 0;
}
char ReadKey() {
  char GetChar;
  RoundBuff.get(&GetChar);
  if (GetChar) {
    return GetChar;
  }
  return -1;
}
