#pragma once

typedef struct {
  int (*init)(int);
  int (*rand)();
} RandomStr;

extern RandomStr Random;
