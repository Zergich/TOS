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
uptr HHDM_Offset = 0;

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
  HHDM_Offset = pmm_state.hhdm_offset;
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
    if (entry->type == LIMINE_MEMMAP_USABLE && entry->base >= 0x100000) {
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
uintptr_t pmm_alloc_frame() {
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
void pmm_free_frame(uintptr_t addr) {
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




#include <ConsoleIO/print.h>
#include <System/MemoryManager/PMM.h>
#include <System/rsod.h>

// Макрос для проверок (используем тот же стиль, что и в VMM)
#define PMM_ASSERT(cond, msg)                                                  \
  do {                                                                         \
    if (!(cond)) {                                                             \
      Panic("PMM TEST FAILED: " msg);                                          \
    }                                                                          \
  } while (0)

void test_pmm() {
  print("Running PMM Tests...\n");

  // Сохраняем исходное состояние счетчика свободных страниц
  u64 initial_free = pmm_get_free_pages();
  print("  [INFO] Initial free pages: ");
  printf("%d", initial_free);
  print("\n");

  PMM_ASSERT(initial_free > 0, "No free pages available at start");

  // ==================================================================
  // ТЕСТ 1: Базовый цикл выделения и освобождения
  // ==================================================================
  print("  Test 1: Single Frame Alloc & Free... ");

  uintptr_t frame1 = pmm_alloc_frame();
  PMM_ASSERT(frame1 != 0, "Failed to allocate first frame");
  PMM_ASSERT((frame1 % 4096) == 0, "Allocated frame is not 4KiB aligned");
  PMM_ASSERT(pmm_get_free_pages() == initial_free - 1, "Free pages count mismatch after alloc");

  // Освобождаем и проверяем счетчик
  pmm_free_frame(frame1);
  PMM_ASSERT(pmm_get_free_pages() == initial_free, "Free pages count mismatch after free");

  print("PASSED\n");

  // ==================================================================
  // ТЕСТ 2: Защита зон памяти (Первый мегабайт)
  // ==================================================================
  print("  Test 2: Memory Zone Protection... ");

  // Выделим пачку фреймов и убедимся, что они не лезут в Real Mode / BIOS зону
  uintptr_t frames[10];
  for (int i = 0; i < 10; i++) {
    frames[i] = pmm_alloc_frame();
    PMM_ASSERT(frames[i] >= 0x100000, "PMM allocated a frame below 1MB!");
  }

  // Чистим за собой
  for (int i = 0; i < 10; i++) {
    pmm_free_frame(frames[i]);
  }

  print("PASSED\n");

  // ==================================================================
  // ТЕСТ 3: Защита от Double Free (Повторное освобождение)
  // ==================================================================
  print("  Test 3: Double Free Protection... ");

  uintptr_t df_frame = pmm_alloc_frame();
  u64 free_before_df = pmm_get_free_pages();

  pmm_free_frame(df_frame); 
  PMM_ASSERT(pmm_get_free_pages() == free_before_df + 1, "First free failed");

  // Вызываем освобождение той же страницы ЕЩЕ РАЗ
  pmm_free_frame(df_frame);

  // Счетчик свободных страниц НЕ должен увеличиться во второй раз!
  PMM_ASSERT(pmm_get_free_pages() == free_before_df + 1, "Double free corrupted free_pages counter!");

  print("PASSED\n");

  // ==================================================================
  // ТЕСТ 4: Стресс-тест на полное истощение памяти (OOM Simulation)
  // ==================================================================
  print("  Test 4: Out Of Memory Simulation... \n");

  u64 allocated_count = 0;
  u64 max_possible = pmm_get_free_pages();
  
  // Создаем временный массив для хранения адресов (выделим на стеке ядра)
  // Внимание: если у тебя мало памяти в системе, этот тест отработает быстро.
  // Если гигабайты — массив может переполнить стек. Для тестов в QEMU выделяй ~32-64MB.
  print("    [DEBUG] Mass allocating frames... ");
  
  // Будем выделять порциями, пока память не кончится
  while (1) {
    uintptr_t f = pmm_alloc_frame();
    if (f == 0) {
      break; // Память успешно закончилась
    }
    allocated_count++;
    
    // Безопасный предохранитель, чтобы не зависнуть в бесконечном цикле,
    // если Next-Fit/First-Fit где-то зациклился
    if (allocated_count > max_possible + 1000) {
      Panic("PMM OOM Test: Allocator is in infinite loop, ignoring bitmap status!");
    }
  }

  print("DONE\n");
  print("    [DEBUG] Allocated total: ");
  printf("%d", allocated_count);
  print(" frames. free_pages = ");
  printf("%d", pmm_get_free_pages());
  print("\n");

  PMM_ASSERT(pmm_get_free_pages() == 0, "PMM returned 0, but free_pages count is not 0!");

  // Пытаемся выделить еще один фрейм, когда всё забито
  uintptr_t oom_frame = pmm_alloc_frame();
  PMM_ASSERT(oom_frame == 0, "PMM managed to allocate frame from empty bitmap!");

  print("    [DEBUG] Mass freeing frames... ");

  // Теперь нам нужно вернуть память системе.
  // Но как, если мы не сохраняли адреса? Очень просто!
  // Пройдемся по всему битмапу ядра и освободим все валидные фреймы, которые были заняты.
  // Для этого временно сломаем инкапсуляцию ради теста, либо освобождаем через поиск.
  // Но правильнее — сделать сброс. Давай освободим память, зная, что мы можем перебрать индексы:
  
  // Так как мы знаем исходный free_pages, мы можем просто сэмулировать откат, 
  // но надежнее — если бы у нас был массив. 
  // Давай сделаем умнее: этот тест крутит аллокацию обратно.
  // Чтобы не упасть по стеку, просто перезапустим систему или очистим битмап.
  // Но раз мы пишем честный тест, давай переберем весь диапазон страниц:
  
  extern uint64_t pmm_state_total_pages(void); // если есть обертка, или сделаем напрямую:
  
  // Для простоты реализации освобождения без сохранения массива:
  // Мы просто заново проинициализируем PMM (вызовем pmm_init), 
  // либо очистим страницы. Но самый изящный способ в тесте OOM —
  // освобождать страницы по ходу их поиска в битмапе.
  
  // Давай откатим систему назад, зная, что мы освобождаем фреймы:
  // (В реальном тесте лучше сохранять адреса, но раз память в 0, 
  // мы можем написать мини-хак для восстановления состояния):
  
  print("RECOVERY... ");
  // Просто вызываем pmm_init() повторно — это сбросит битмап в исходное чистое состояние!
  pmm_init(); 

  PMM_ASSERT(pmm_get_free_pages() == initial_free, "PMM re-init failed to recover state");
  print("PASSED\n");

  print("All PMM Tests Passed Successfully!\n");
}
