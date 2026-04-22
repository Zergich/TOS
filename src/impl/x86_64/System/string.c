#include <stdint.h>
#include <string.h>

typedef enum {
  ERROR_CONVERT = -1,
  OK = 0,

} StringOPError;

void to_lowercase(char *str);
int strcmp(const char *s1, const char *s2);
unsigned int strlen(char *string);
int IsEmptyOrWhitespace(const char *str);
IntConvertResult
StringToInt(char *string); // коспилятор сука не дает из за того что Atoi
                           // название функции зарезервированно
char IsDigit(char *string);

StringStruct string = {
    .ToLower = to_lowercase,
    .Strcmp = strcmp,
    .Strlen = strlen,
    .Atoi = StringToInt,
    .IsDigit = IsDigit,
    .IsEmptyOrWhitespace = IsEmptyOrWhitespace,
};

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

// Возвращает число, соответствующее символу (цифре)
char smtoi(char symbol) {
  switch (symbol) {
  case '0':
    return 0;
  case '1':
    return 1;
  case '2':
    return 2;
  case '3':
    return 3;
  case '4':
    return 4;
  case '5':
    return 5;
  case '6':
    return 6;
  case '7':
    return 7;
  case '8':
    return 8;
  case '9':
    return 9;
  default:
    return 0;
  }
}
// Проверка строки на то, является ли она числом
char IsDigit(char *string) {
  // пустая строка
  if (string[0] == '\0')
    return 0;
  // бегаем пока не конец строки
  for (int i = 0; string[i] != '\0'; i++) {
    char symbol = string[i];
    // если вдруг отрицательное, то корректная проверка
    if (i == 0 && symbol == '-' && string[i + 1] != '\0')
      continue;
    // проверка на то, входит ли символ в диапазон цифр
    if (symbol < '0' || symbol > '9') {
      return 0; // Не число
    }
  }
  return 1; // Все таки числох
}

IntConvertResult StringToInt(char *string) {
  IntConvertResult result;
  if (!IsDigit(string)) {
    result.error = ERROR_CONVERT;
    result.value = -1;
    return result;
  }

  char minus = 0;
  if (string[0] == '-')
    minus = 1;

  int i_result = 0;
  int umn = 1;
  short len = strlen(string);
  for (int i = len - 1; i >= minus; i--) {
    char symbol = string[i];
    char digit = smtoi(symbol);
    i_result = (digit * umn) + i_result;
    umn *= 10;
  }
  if (minus)
    i_result *= -1;

  result.error = OK;
  result.value = i_result;
  return result;
}

int IsEmptyOrWhitespace(const char *str) {
  if (str == 0)
    return 1;
  while (*str) {
    if (*str != ' ' && *str != '\t' && *str != '\n' && *str != '\r') {
      return 0;
    }
    str++;
  }

  return 1;
}
