#pragma once

// Структура-результат для типа int
typedef struct {
  int value;
  int error;
} IntConvertResult;

typedef struct {
  void (*ToLower)(char *str);
  int (*Strcmp)(const char *s1, const char *s2);
  unsigned int (*Strlen)(char *string);
  IntConvertResult (*Atoi)(char *string);
  char (*IsDigit)(char *string);
  int (*IsEmptyOrWhitespace)(const char *str);
} StringStruct;

// void to_lowercase(char *str);
// int strcmp(const char *s1, const char *s2);
// unsigned int strlen(char *string);
// IntResult Atoi(char *string);
// char IsDigit(char *string);
