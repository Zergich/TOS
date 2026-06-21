#pragma once
#include "types.h"
#include <limine.h>

extern char *VersionOS;
extern char *ShellVersion;

// можно использовать и там и там я все равно консоль хочу
// консоль
extern u16 NUM_COLUMS;
extern u16 NUM_ROWS;

// экран и графика
extern u16 HeightDisplay;
extern u64 BufferPitchW;
extern u16 WidthDisplay;

struct MemoryType {
  u64 UsableMemory;
  u64 ReservedMemory;
};
extern struct MemoryType ReturnMemoryMapStruct;
extern volatile struct limine_hhdm_request *HHDMRequest;
extern volatile struct limine_memmap_request *MemMapStructPtr;
extern uptr HHDM_Offset;
struct MemoryType ReturnMemoryMap();
