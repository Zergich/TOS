#pragma once
#include "types.h"
#include <stddef.h>

extern char *VersionOS;
extern const u8 NUM_COLUMS;
extern const u8 NUM_ROWS;

extern u64 Mb2InfoAddr;
extern u32 Mb2Magic;

void PrintMem();
