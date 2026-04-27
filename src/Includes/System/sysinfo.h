#pragma once
#include "types.h"
#include <stddef.h>

// херь для определение списка параметров для разных режимов ядра
// TEXT = 0
// GRAPHICS = 1
#define REGIME 1

// GRUB = 0
// LIMINE = 1
#define LOADER 1

extern char *VersionOS;

// можно использовать и там и там я все равно консоль хочу
extern const u8 NUM_COLUMS;
extern const u8 NUM_ROWS;

#if LOADER == 0
extern u64 Mb2InfoAddr;
extern u32 Mb2Magic;
#endif

void PrintMem();
