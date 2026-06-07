#pragma once

#include <System/sysinfo.h>
#include <stdbool.h>
#include <stdint.h>
#include <types.h>

#define PAGE_SIZE 4096

// Флаги Page Table Entry (биты 0-11)
// NX (No Execute) - бит 63 (задается отдельно через сдвиг)
#define PTE_PRESENT (1ULL << 0)
#define PTE_WRITABLE (1ULL << 1)
#define PTE_USER (1ULL << 2)
#define PTE_ACCESSED                                                           \
  (1ULL << 5) // Процессор ставит этот флаг, когда читает страницу
#define PTE_DIRTY                                                              \
  (1ULL << 6) // Процессор ставит этот флаг, когда пишет в страницу
#define PTE_NX (1ULL << 63) // Запрет выполнения кода

// Маска для очистки флагов и получения чистого физического адреса
#define PTE_ADDR_MASK 0x000FFFFFFFFFF000ULL

// Извлечение индексов из виртуального адреса (9 бит на каждый уровень)
#define PML4_INDEX(addr) (((addr) >> 39) & 0x1FF)
#define PDPT_INDEX(addr) (((addr) >> 30) & 0x1FF)
#define PD_INDEX(addr) (((addr) >> 21) & 0x1FF)
#define PT_INDEX(addr) (((addr) >> 12) & 0x1FF)

// Тот самый HHDM (Higher-Half Direct Map)
// Загрузчик (Limine) мапирует физическую память начиная с этого виртуального
// адреса Значение берется из лимитеров Limine, для примера возьмем стандартное

// Макрос для перехода от физического адреса к виртуальному (через HHDM)
#define PHYS_TO_VIRT(phys) ((void *)((u64)(phys) + HHDM_Offset))

// Структура, описывающая адресное пространство процесса
typedef struct address_space {
  u64 pml4_phys; // Физический адрес корневой таблицы (для записи в CR3)
  // В реальном ядре здесь еще будут списки регионов памяти (VMA),
  // статистика использования и блокировки (spinlocks).
} address_space_t;
extern address_space_t kernel_space; // Инициализируем нулем

void vmm_map_page(address_space_t *space, uint64_t virt, uint64_t phys,
                  uint64_t flags);
static void clear_page(uint64_t *virt_addr);
void vmm_switch_space(address_space_t *space);
void vmm_unmap_page(address_space_t *space, u64 virt);
void vmm_init();

//----------------------
uint64_t vmm_virt_to_phys(address_space_t *space, uint64_t virt);
void test_vmm();
