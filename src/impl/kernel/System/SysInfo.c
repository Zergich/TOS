#include <ConsoleIO/print.h>
#include <System/sysinfo.h>
#include <limine.h>
#include <types.h>

struct MemoryType ReturnMemoryMapStruct = {.ReservedMemory = 0,
                                           .UsableMemory = 0};

struct MemoryType ReturnMemoryMap() {

  if (MemMapStructPtr->response == NULL) {
    return ReturnMemoryMapStruct;
  }

  uint64_t total_bytes = 0;

  uint64_t total_bytesRES = 0;
  u64 ConvertToMB = 1024 * 1024;

  struct limine_memmap_response *response = MemMapStructPtr->response;

  // Перебираем все записи в карте памяти
  for (uint64_t i = 0; i < response->entry_count; i++) {
    struct limine_memmap_entry *entry = response->entries[i];

    if (entry->type == LIMINE_MEMMAP_USABLE) {
      total_bytes += entry->length;
    }
    if (entry->type == LIMINE_MEMMAP_RESERVED) {
      total_bytesRES += entry->length;
    }
  }
  ReturnMemoryMapStruct.UsableMemory = total_bytes / ConvertToMB;
  ReturnMemoryMapStruct.ReservedMemory = total_bytesRES / ConvertToMB;
  return ReturnMemoryMapStruct;
}


// Надежная обертка над CPUID для x86_64
static inline void cpuid(uint32_t code, uint32_t subcode, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    asm volatile (
        "cpuid"
        : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
        : "a"(code), "c"(subcode)
    );
}
cpu_info_t CPUInfo;
void PintInfoCPU()
{

    // Выводим информацию
    printf("CPU Vendor: %s\n", CPUInfo.vendor);
    printf("CPU Brand : %s\n", CPUInfo.brand);
    printf("Family: %u, Model: %u, Stepping: %u\n", CPUInfo.family, CPUInfo.model, CPUInfo.stepping);
    
    printf("Features: ");
    if (CPUInfo.has_APIC)   printf("APIC ");
    if (CPUInfo.has_SSE)    printf("SSE ");
    if (CPUInfo.has_SSE2)   printf("SSE2 ");
    if (CPUInfo.has_AVX)    printf("AVX ");
    if (CPUInfo.has_NX)     printf("NX(No-Execute) ");
    if (CPUInfo.has_RDRAND) printf("RDRAND ");
  print("\n");

}

void cpu_init(cpu_info_t *info) {
    uint32_t eax, ebx, ecx, edx;

    // --- 1. Получаем Vendor ID (Функция 0) ---
    cpuid(0, 0, &eax, &ebx, &ecx, &edx);
    
    // Порядок регистров для Vendor ID специфичен: EBX, EDX, ECX
    ((uint32_t*)info->vendor)[0] = ebx;
    ((uint32_t*)info->vendor)[1] = edx;
    ((uint32_t*)info->vendor)[2] = ecx;
    info->vendor[12] = '\0';

    // Максимальный поддерживаемый уровень базовых функций CPUID
    uint32_t max_basic_leaf = eax;

    // --- 2. Получаем Family, Model, Stepping и Feature Flags (Функция 1) ---
    if (max_basic_leaf >= 1) {
        cpuid(1, 0, &eax, &ebx, &ecx, &edx);

        // Парсим версию процессора
        info->stepping = eax & 0xF;
        info->model = (eax >> 4) & 0xF;
        info->family = (eax >> 8) & 0xF;

        // Если семейство 0xF или 0x6, модель и семейство расширяются (требование спецификации Intel/AMD)
        if (info->family == 0xF || info->family == 0x6) {
            info->model += ((eax >> 16) & 0xF) << 4;
        }
        if (info->family == 0xF) {
            info->family += (eax >> 20) & 0xFF;
        }

        // Парсим фичи (Features)
        info->has_APIC   = (edx & (1 << 9))  != 0;
        info->has_SSE    = (edx & (1 << 25)) != 0;
        info->has_SSE2   = (edx & (1 << 26)) != 0;
        info->has_AVX    = (ecx & (1 << 28)) != 0;
        info->has_RDRAND = (ecx & (1 << 30)) != 0;
    }

    // --- 3. Получаем расширенные фичи (Функция 0x80000000) ---
    cpuid(0x80000000, 0, &eax, &ebx, &ecx, &edx);
    uint32_t max_extended_leaf = eax;

    if (max_extended_leaf >= 0x80000001) {
        cpuid(0x80000001, 0, &eax, &ebx, &ecx, &edx);
        // NX-бит (No-Execute) находится в EDX расширенной функции 1, бит 20
        info->has_NX = (edx & (1 << 20)) != 0;
    }

    // --- 4. Получаем красивое имя процессора (Brand String, Функции 0x80000002 - 0x80000004) ---
    if (max_extended_leaf >= 0x80000004) {
        uint32_t *brand_ptr = (uint32_t*)info->brand;
        
        // Нам нужно вызвать CPUID три раза подряд, каждый раз собирая по 16 байт
        for (uint32_t leaf = 0x80000002; leaf <= 0x80000004; leaf++) {
            cpuid(leaf, 0, &brand_ptr[0], &brand_ptr[1], &brand_ptr[2], &brand_ptr[3]);
            brand_ptr += 4; // Сдвигаемся на 16 байт вперед
        }
        info->brand[48] = '\0';
    } else {
        // Если процессор древний и не поддерживает Brand String
        for(int i = 0; i < 48; i++) info->brand[i] = ' ';
        info->brand[48] = '\0';
    }
}
