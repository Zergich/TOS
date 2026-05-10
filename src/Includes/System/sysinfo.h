#pragma once
#include "types.h"
#include <limine.h>

extern char *VersionOS;

// можно использовать и там и там я все равно консоль хочу
extern u16 NUM_COLUMS;
extern u16 NUM_ROWS;

extern u64 HeightDisplay;
extern u64 BufferPitchW;

extern u64 Mb2InfoAddr;
extern u32 Mb2Magic;

struct MemoryType {
  u64 UsableMemory;
  u64 ReservedMemory;
};
extern struct MemoryType ReturnMemoryMapStruct;
extern volatile struct limine_memmap_request *MemMapStructPtr;
struct MemoryType ReturnMemoryMap();
