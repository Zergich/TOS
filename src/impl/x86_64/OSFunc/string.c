#include <stdint.h>
#include <string.h>

unsigned int strlen(char *string) {
  uint32_t Length = 0;
  while (string[Length] != '\0')
    Length++;
  return Length;
}

void to_lowercase(char *str) {
  for (; *str; str++) {
    if (*str >= 'A' && *str <= 'Z') {
      *str = *str + 32;
    }
  }
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && *s2) {
    char c1 = (*s1 >= 'A' && *s1 <= 'Z') ? *s1 + 32 : *s1;
    char c2 = (*s2 >= 'A' && *s2 <= 'Z') ? *s2 + 32 : *s2;
    if (c1 != c2)
      return c1 - c2;
    s1++;
    s2++;
  }
  char c1 = (*s1 >= 'A' && *s1 <= 'Z') ? *s1 + 32 : *s1;
  char c2 = (*s2 >= 'A' && *s2 <= 'Z') ? *s2 + 32 : *s2;
  return c1 - c2;
}
