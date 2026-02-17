#include <stdint.h>
#include <string.h>

unsigned int strlen(char *string) {
  uint32_t Length = 0;
  while (string[Length] != '\0')
    Length++;
  return Length;
}
