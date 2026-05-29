#include <System/MemoryManager/PMM.h>
#include <System/rsod.h>
#include <System/sysinfo.h>
#include <libs/MemoryUtils.h>
#include <limine.h>
#include <stddef.h>
#include <types.h>

// Спинлок для многопоточности
// чтоб два барана в одну дырку не лезли
typedef struct {
  volatile int locked;
} spinlock_t;

static inline void spinlock_lock(spinlock_t *lock) {
  while (__sync_bool_compare_and_swap(&lock->locked, 0, 1)) {
    __asm__ volatile("pause" ::: "memory");
  }
}

static inline void spinlock_unlock(spinlock_t *lock) {
  __asm__ volatile("" ::: "memory");
  lock->locked = 0;
}
static struct {
  spinlock_t lock;
  u64 *bitmap;        // Указатель на массив битмапа (в виртуальной памяти HHDM)
  u64 total_pages;    // Всего страниц в системе
  u64 free_pages;     // Свободных страниц
  u64 last_alloc_idx; // Индекс последней выделенной страницы (для
                      // оптимизации next-fit)
  uintptr_t hhdm_offset; // Смещение HHDM
} pmm_state;

#define BITS_PER_QWORD 64
#define BIT_INDEX(addr) ((addr) / PAGE_SIZE)
#define QWORD_INDEX(bit_idx) ((bit_idx) / BITS_PER_QWORD)
#define BIT_OFFSET(bit_idx) ((bit_idx) % BITS_PER_QWORD)

// функции работы с битами чтоб не переберать весь набор и так далее
// еще прикол 1NULL это единица в 64 битном формате ансигнед лонг лонг иначе
// вылетет за границы и поломается
static inline void bitmap_set(u64 bit_idx) {
  pmm_state.bitmap[QWORD_INDEX(bit_idx)] |= (1ULL << BIT_OFFSET(bit_idx));
}

static inline void bitmap_clear(u64 bit_idx) {
  pmm_state.bitmap[QWORD_INDEX(bit_idx)] &= ~(1ULL << BIT_OFFSET(bit_idx));
}

static inline int bitmap_test(u64 bit_idx) {
  return (pmm_state.bitmap[QWORD_INDEX(bit_idx)] &
          (1ULL << BIT_OFFSET(bit_idx))) != 0;
}

void pmm_init() {
  if (MemMapStructPtr->response == NULL || HHDMRequest->response == NULL)
    PMMError();

  struct limine_hhdm_response *hhdm_response = HHDMRequest->response;
  pmm_state.hhdm_offset = hhdm_response->offset;
  struct limine_memmap_response *memmap = MemMapStructPtr->response;

  // для расчета макс размера битмапа
  u64 max_addr = 0;
  for (u64 i = 0; i < memmap->entry_count; i++) {
    struct limine_memmap_entry *entry = memmap->entries[i];

    if (entry->type == LIMINE_MEMMAP_USABLE ||
        entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {

      u64 top = entry->base + entry->length;
      if (top > max_addr)
        max_addr = top;
    }
  }

  pmm_state.total_pages = max_addr / PAGE_SIZE;
  u64 bitmap_size_bytes = (pmm_state.total_pages + 7) / 8; // Округляем вверх
  u64 bitmap_size_pages = (bitmap_size_bytes + PAGE_SIZE - 1) / PAGE_SIZE;

  u64 bitmap_phys = 0;
  for (u64 i = 0; i < memmap->entry_count; i++) {
    struct limine_memmap_entry *entry = memmap->entries[i];
    if (entry->type == LIMINE_MEMMAP_USABLE &&
        entry->length >= bitmap_size_bytes) {
      bitmap_phys = entry->base;
      break;
    }
  }

  if (bitmap_phys == 0)
    PMMBitMapPlaceNotfound();

  // Отображаем битмап в виртуальную память через HHDM
  pmm_state.bitmap = (u64 *)(bitmap_phys + pmm_state.hhdm_offset);

  // Инициализация битмапа + разметка под 0 (занята)
  memset(pmm_state.bitmap, 0, bitmap_size_bytes);

  // Разметка и поиск свободных страниц
  pmm_state.free_pages = 0;
  for (u64 i = 0; i < memmap->entry_count; i++) {
    struct limine_memmap_entry *entry = memmap->entries[i];
    if (entry->type == LIMINE_MEMMAP_USABLE) {
      u64 start_idx = BIT_INDEX(entry->base);
      u64 end_idx = BIT_INDEX(entry->base + entry->length);

      for (u64 j = start_idx; j < end_idx; j++) {
        bitmap_set(j);
        pmm_state.free_pages++;
      }
    }
  }
  // Резервирование памяти для самого битмапа
  u64 bitmap_start_idx = BIT_INDEX(bitmap_phys);
  for (u64 i = 0; i < bitmap_size_pages; i++) {
    if (bitmap_test(bitmap_start_idx + i)) {
      bitmap_clear(bitmap_start_idx + i);
      pmm_state.free_pages--;
    }
  }

  pmm_state.last_alloc_idx = 0;
}
uintptr_t pmm_alloc_page(void) {
  spinlock_lock(&pmm_state.lock);

  // поиск свободной страницы по последнему индексу
  for (u64 i = pmm_state.last_alloc_idx; i < pmm_state.total_pages; i++) {
    if (bitmap_test(i)) {
      bitmap_clear(i); // Помечаем как занятую (0)
      pmm_state.free_pages--;
      pmm_state.last_alloc_idx = i + 1;

      uintptr_t phys_addr = i * PAGE_SIZE;
      spinlock_unlock(&pmm_state.lock);

      // преобразование виртуального адреса от лима1н в физический и само
      // выделение страницых
      memset((void *)(phys_addr + pmm_state.hhdm_offset), 0, PAGE_SIZE);

      return phys_addr;
    }
  }

  // Если дошли до конца, ищем с начала (First-Fit)
  for (u64 i = 0; i < pmm_state.last_alloc_idx; i++) {
    if (bitmap_test(i)) {
      bitmap_clear(i);
      pmm_state.free_pages--;
      pmm_state.last_alloc_idx = i + 1;

      uintptr_t phys_addr = i * PAGE_SIZE;
      spinlock_unlock(&pmm_state.lock);

      memset((void *)(phys_addr + pmm_state.hhdm_offset), 0, PAGE_SIZE);
      return phys_addr;
    }
  }

  spinlock_unlock(&pmm_state.lock);
  return 0;
}

// освобождение страницы
void pmm_free_page(uintptr_t addr) {
  if (addr == 0)
    return; // Нельзя освободить нулевую страницу

  u64 bit_idx = BIT_INDEX(addr);

  spinlock_lock(&pmm_state.lock);

  // Если бит уже 1 (страница свободна), просто выходим, не трогая счетчик!
  if (bitmap_test(bit_idx)) {
    spinlock_unlock(&pmm_state.lock);
    return;
  }

  // Если бит был 0 (страница занята), помечаем как свободную
  bitmap_set(bit_idx);
  pmm_state.free_pages++;

  spinlock_unlock(&pmm_state.lock);
}

u64 pmm_get_free_pages(void) { return pmm_state.free_pages; }
