#pragma once

#include <types.h>
// Структура-результат для типа int
typedef struct {
  int value;
  int error;
} IntConvertResult;

typedef struct {
  void (*ToLower)(u32 *str);
  int (*Strcmp)(const u32 *s1, const u32 *s2);
  unsigned int (*Strlen)(const u32 *string);
  unsigned int (*StrlenC)(const char *string);
  IntConvertResult (*Atoi)(const u32 *string);
  int (*IsDigit)(const u32 *string);
  int (*IsEmptyOrWhitespace)(const u32 *str);
  u32 **(*Split)(u32 *str, u32 delimiter, int *out_argc);
  void (*Strcpy)(const char *source, char *dest);
  char **(*SplitCh)(char *str, char delimiter, int *out_argc);

} StringStruct;
extern StringStruct string;
// void to_lowercase(char *str);
// int strcmp(const char *s1, const char *s2);
// unsigned int strlen(char *string);
// IntResult Atoi(char *string);
// char IsDigit(char *string);
//
char *strchr(const char *s, int c); // для fatFS
