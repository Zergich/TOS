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



// Инфа о процессоре
typedef struct {
    char vendor[13];       // Имя производителя (например, "GenuineIntel")
    char brand[49];        // Полное название (например, "Intel(R) Core(TM) i7...")
    u32 stepping;     // Степпинг (ревизия)
    u32 model;        // Модель
    u32 family;       // Семейство
    
    // Флаги важных фич процессора
    bool has_APIC;
    bool has_SSE;
    bool has_SSE2;
    bool has_AVX;
    bool has_NX;           // No-Execute бит (защита страниц памяти от исполнения кода)
    bool has_RDRAND;       // Аппаратный генератор случайных чисел
} cpu_info_t;
extern cpu_info_t CPUInfo;
// Функция для инициализации и сбора инфы
void cpu_init(cpu_info_t *info);
void PintInfoCPU();
