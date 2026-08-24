#include <System/MemoryManager/kmalloc/kmalloc.h>
#include <libs/string.h>
#include <stddef.h>
#include <stdint.h>
#include <types.h>
typedef enum {
  ERROR_CONVERT = -1,
} StringOPError;

// Обновленные прототипы функций
void to_lowercase(u32 *str);
int strcmp(const u32 *s1, const u32 *s2);
unsigned int strlen(const u32 *string);
int IsEmptyOrWhitespace(const u32 *str);
IntConvertResult StringToInt(const u32 *string);
int IsDigit(const u32 *string);
u32 **SplitStr(u32 *str, u32 delimiter, int *out_argc);
void strcpy(const u32 *source, u32 *dest);
char **SplitStrChar(char *str, char delimiter, int *out_argc);

// Инициализация структуры
StringStruct string = {
    .ToLower = to_lowercase,
    .Strcmp = strcmp,
    .Strlen = strlen,
    .Atoi = StringToInt,
    .IsDigit = IsDigit,
    .IsEmptyOrWhitespace = IsEmptyOrWhitespace,
    .Split = SplitStr,
    .Strcpy = strcpy,
    .SplitCh = SplitStrChar,
};

unsigned int strlen(const u32 *string) {
  uint32_t Length = 0;
  while (string[Length] != 0) // '\0' это просто 0
    Length++;
  return Length;
}

void to_lowercase(u32 *str) {
  for (; *str; str++) {
    // Английские буквы
    if (*str >= 'A' && *str <= 'Z') {
      *str = *str + 32;
    }
    // ЗАДЕЛ НА БУДУЩЕЕ: Если захочешь сделать lowercase для русских букв
    // (ведь u32 это позволяет), то это будет выглядеть примерно так:
    // else if (*str >= 0x0410 && *str <= 0x042F) {
    //   *str = *str + 32; // Смещение в таблице Unicode для кириллицы
    // }
  }
}

int strcmp(const u32 *s1, const u32 *s2) {
  while (*s1 && *s2) {
    u32 c1 = (*s1 >= 'A' && *s1 <= 'Z') ? *s1 + 32 : *s1;
    u32 c2 = (*s2 >= 'A' && *s2 <= 'Z') ? *s2 + 32 : *s2;
    if (c1 != c2)
      return (int)(c1 - c2); // Каст в int обязателен для u32
    s1++;
    s2++;
  }
  u32 c1 = (*s1 >= 'A' && *s1 <= 'Z') ? *s1 + 32 : *s1;
  u32 c2 = (*s2 >= 'A' && *s2 <= 'Z') ? *s2 + 32 : *s2;
  return (int)(c1 - c2);
}

// Возвращает число, соответствующее символу (цифре)
// Оптимизировано: switch больше не нужен
int smtoi(u32 symbol) {
  if (symbol >= '0' && symbol <= '9') {
    return symbol - '0';
  }
  return 0;
}

// Проверка строки на то, является ли она числом
int IsDigit(const u32 *string) {
  if (string[0] == 0) // пустая строка
    return 0;

  for (int i = 0; string[i] != 0; i++) {
    u32 symbol = string[i];
    // Пропуск знака минус в начале
    if (i == 0 && symbol == '-' && string[i + 1] != 0)
      continue;

    // проверка на то, входит ли символ в диапазон цифр
    if (symbol < '0' || symbol > '9') {
      return 0; // Не число
    }
  }
  return 1; // Все таки число
}

IntConvertResult StringToInt(const u32 *string) {
  IntConvertResult result;
  if (!IsDigit(string)) {
    result.error = ERROR_CONVERT;
    result.value = -1;
    return result;
  }

  int minus = 0;
  if (string[0] == '-')
    minus = 1;

  int i_result = 0;
  int umn = 1;
  int len = strlen(string);

  for (int i = len - 1; i >= minus; i--) {
    u32 symbol = string[i];
    int digit = smtoi(symbol);
    i_result = (digit * umn) + i_result;
    umn *= 10;
  }
  if (minus)
    i_result *= -1;

  result.error = OK;
  result.value = i_result;
  return result;
}

int IsEmptyOrWhitespace(const u32 *str) {
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

int is_space(u32 c) {
  return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

void strcpy(const u32 *source, u32 *dest) {
  int i = 0;
  while (source[i] != 0) {
    dest[i] = source[i];
    i++;
  }
  dest[i] = 0;
}

char **SplitStrChar(char *str, char delimiter, int *out_argc) {
  int count = 0;
  int in_token = 0;

  // 1. Первый проход: считаем количество токенов
  for (int i = 0; str[i] != '\0'; i++) {
    if (str[i] == delimiter) {
      in_token = 0;
    } else if (!in_token) {
      count++;
      in_token = 1;
    }
  }

  *out_argc = count;
  if (count == 0)
    return NULL;

  // 2. Выделяем память под массив указателей char*
  char **argv = (char **)kmalloc(sizeof(char *) * (count + 1));
  if (!argv)
    return NULL;

  // 3. Второй проход: заменяем разделители на '\0' и сохраняем указатели
  int argc = 0;
  in_token = 0;
  for (int i = 0; str[i] != '\0'; i++) {
    if (str[i] == delimiter) {
      str[i] = '\0';
      in_token = 0;
    } else if (!in_token) {
      argv[argc++] = &str[i];
      in_token = 1;
    }
  }
  argv[argc] = NULL;
  return argv;
}

// Функция возвращает массив указателей (u32**)
u32 **SplitStr(u32 *str, u32 delimiter, int *out_argc) {
  int count = 0;
  int in_token = 0;

  // 1. Первый проход: считаем количество слов
  for (int i = 0; str[i] != 0; i++) {
    if (str[i] == delimiter) {
      in_token = 0;
    } else if (!in_token) {
      count++;
      in_token = 1;
    }
  }

  *out_argc = count;
  if (count == 0)
    return NULL;

  // 2. Выделяем память под массив указателей u32*
  u32 **argv = (u32 **)kmalloc(sizeof(u32 *) * (count + 1));
  if (!argv)
    return NULL; // Ошибка аллокации

  // 3. Второй проход: записываем указатели
  int argc = 0;
  in_token = 0;
  for (int i = 0; str[i] != 0; i++) {
    if (str[i] == delimiter) {
      str[i] = 0; // Заменяем разделитель на терминатор для u32
      in_token = 0;
    } else if (!in_token) {
      argv[argc++] = &str[i];
      in_token = 1;
    }
  }
  argv[argc] = NULL; // Терминатор массива указателей
  return argv;
}
