#include <System/MemoryManager/PMM.h>
#include <System/MemoryManager/VMM.h>
#include <types.h>

#include <ConsoleIO/print.h>
#include <System/rsod.h>
// Внешняя функция от менеджера физической памяти (PMM)
// Возвращает физический адрес свободного блока на 4096 байт

// Функция очистки выделенной таблицы (заполняет нулями)
static void clear_page(u64 *virt_addr) {
  for (int i = 0; i < 512; i++) {
    virt_addr[i] = 0;
  }
}

// ГЛАВНАЯ ФУНКЦИЯ: Мапирует виртуальный адрес на физический
#define PTE_HUGE (1ULL << 7) // Бит размера страницы (2MB или 1GB)
// Базовые маски флагов для x86_64
#define x86_PTE_PRESENT  (1ULL << 0)
#define x86_PTE_WRITABLE (1ULL << 1)
#define x86_PTE_USER     (1ULL << 2)
#define x86_PTE_NX       (1ULL << 63)

void vmm_map_page(address_space_t *space, uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t *pml4 = (uint64_t *)PHYS_TO_VIRT(space->pml4_phys);
    uint64_t new_table_phys;

    // --- 1. УРОВЕНЬ: PML4 ---> PDPT ---
    uint64_t pml4_idx = PML4_INDEX(virt);
    if (!(pml4[pml4_idx] & x86_PTE_PRESENT)) {
        new_table_phys = pmm_alloc_frame();
        if (new_table_phys == 0) return;
        
        // ФИКС: Очищаем новую страницу, чтобы там не было мусора!
        clear_page((u64 *)PHYS_TO_VIRT(new_table_phys));
        
        pml4[pml4_idx] = new_table_phys | x86_PTE_PRESENT | x86_PTE_WRITABLE | x86_PTE_USER;
    }
    uint64_t *pdpt = (uint64_t *)PHYS_TO_VIRT(pml4[pml4_idx] & PTE_ADDR_MASK);

    // --- 2. УРОВЕНЬ: PDPT ---> PD ---
    uint64_t pdpt_idx = PDPT_INDEX(virt);
    if (!(pdpt[pdpt_idx] & x86_PTE_PRESENT)) {
        new_table_phys = pmm_alloc_frame();
        if (new_table_phys == 0) return;
        
        // ФИКС: Очищаем новую страницу!
        clear_page((u64 *)PHYS_TO_VIRT(new_table_phys));
        
        pdpt[pdpt_idx] = new_table_phys | x86_PTE_PRESENT | x86_PTE_WRITABLE | x86_PTE_USER;
    } else {
        if (pdpt[pdpt_idx] & (1ULL << 7)) return; 
    }
    uint64_t *pd = (uint64_t *)PHYS_TO_VIRT(pdpt[pdpt_idx] & PTE_ADDR_MASK);

    // --- 3. УРОВЕНЬ: PD ---> PT ---
    uint64_t pd_idx = PD_INDEX(virt);
    if (!(pd[pd_idx] & x86_PTE_PRESENT)) {
        new_table_phys = pmm_alloc_frame();
        if (new_table_phys == 0) return;
        
        // ФИКС: Очищаем новую страницу!
        clear_page((u64 *)PHYS_TO_VIRT(new_table_phys));
        
        pd[pd_idx] = new_table_phys | x86_PTE_PRESENT | x86_PTE_WRITABLE | x86_PTE_USER;
    } else {
        if (pd[pd_idx] & (1ULL << 7)) return;
    }
    uint64_t *pt = (uint64_t *)PHYS_TO_VIRT(pd[pd_idx] & PTE_ADDR_MASK);

    // --- 4. УРОВЕНЬ: Финальный лист (PT ---> Физический фрейм) ---
    uint64_t pt_idx = PT_INDEX(virt);

    // Применяем флаги
    pt[pt_idx] = (phys & PTE_ADDR_MASK) | flags | x86_PTE_PRESENT;

    // Сбрасываем TLB
    __asm__ volatile("invlpg (%0)" ::"r"(virt) : "memory");
}

// Функция переключения адресного пространства
void vmm_switch_space(address_space_t *space) {
  __asm__ volatile("mov %0, %%cr3" // Записываем физ. адрес PML4 в CR3
                   ::"r"(space->pml4_phys)
                   : "memory");
}
address_space_t kernel_space = {0};
void vmm_init() {
  // 1. Читаем текущий физический адрес PML4 из регистра CR3
  // Limine уже настроил его для нас перед передачей управления ядру
  u64 current_cr3;
  __asm__ volatile("mov %%cr3, %0" : "=r"(current_cr3));

  // 2. Сохраняем его в нашу глобальную структуру
  // (Обязательно применяем маску, чтобы очистить биты флагов CR3,
  // оставив только чистый физический адрес)
  kernel_space.pml4_phys = current_cr3 & 0x000FFFFFFFFFF000ULL;

  // Теперь kernel_space указывает на реально работающее адресное пространство!

  // ... дальнейшая инициализация VMM ...
}

// Отвязывает виртуальный адрес от физического фрейма.
// ВНИМАНИЕ: Эта функция НЕ освобождает сам физический фрейм через PMM!
// Она только удаляет запись из таблиц страниц.
void vmm_unmap_page(address_space_t *space, u64 virt) {
  u64 *pml4 = (u64 *)PHYS_TO_VIRT(space->pml4_phys);

  // Спускаемся по таблицам (идем до PT)
  u64 pml4_entry = pml4[PML4_INDEX(virt)];
  if (!(pml4_entry & PTE_PRESENT))
    return; // Уже не замаплено
  u64 *pdpt = (u64 *)PHYS_TO_VIRT(pml4_entry & PTE_ADDR_MASK);

  u64 pdpt_entry = pdpt[PDPT_INDEX(virt)];
  if (!(pdpt_entry & PTE_PRESENT))
    return;
  u64 *pd = (u64 *)PHYS_TO_VIRT(pdpt_entry & PTE_ADDR_MASK);

  u64 pd_entry = pd[PD_INDEX(virt)];
  if (!(pd_entry & PTE_PRESENT))
    return;
  u64 *pt = (u64 *)PHYS_TO_VIRT(pd_entry & PTE_ADDR_MASK);

  u64 pt_index = PT_INDEX(virt);

  // Если страница уже не присутствует, ничего не делаем
  if (!(pt[pt_index] & PTE_PRESENT))
    return;

  // СТИРАЕМ ЗАПИСЬ! (Устанавливаем в 0)
  pt[pt_index] = 0;

  // КРИТИЧЕСКИ ВАЖНО: Сбрасываем кэш TLB для этого адреса!
  // Иначе процессор продолжит использовать старый кэш и писать в ту же память.
  __asm__ volatile("invlpg (%0)" ::"r"(virt) : "memory");
}

//-------------------------------------------------------------------=====
// Вспомогательная функция для тестов: возвращает физический адрес
// по виртуальному. Возвращает 0, если страница не Present.
u64 vmm_virt_to_phys(address_space_t *space, u64 virt) {
  u64 *pml4 = (u64 *)PHYS_TO_VIRT(space->pml4_phys);

  u64 pml4_entry = pml4[PML4_INDEX(virt)];
  if (!(pml4_entry & PTE_PRESENT))
    return 0;

  u64 *pdpt = (u64 *)PHYS_TO_VIRT(pml4_entry & PTE_ADDR_MASK);
  u64 pdpt_entry = pdpt[PDPT_INDEX(virt)];
  if (!(pdpt_entry & PTE_PRESENT))
    return 0;

  u64 *pd = (u64 *)PHYS_TO_VIRT(pdpt_entry & PTE_ADDR_MASK);
  u64 pd_entry = pd[PD_INDEX(virt)];
  if (!(pd_entry & PTE_PRESENT))
    return 0;

  u64 *pt = (u64 *)PHYS_TO_VIRT(pd_entry & PTE_ADDR_MASK);
  u64 pt_entry = pt[PT_INDEX(virt)];
  if (!(pt_entry & PTE_PRESENT))
    return 0;

  // Возвращаем физический адрес фрейма + смещение внутри страницы
  return (pt_entry & PTE_ADDR_MASK) | (virt & 0xFFF);
}

// Макрос для прерывания тестов при ошибке
#define ASSERT(cond, msg)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      Panic("VMM TEST FAILED: " msg);                                          \
    }                                                                          \
  } while (0)

// Вспомогательная переменная для тестов (выбор безопасного виртуального адреса)
// ВАЖНО: Этот адрес не должен пересекаться с ядром, HHDM и видео памятью!
// Безопасные адреса в стандартном регионе ядра (PML5 entry 270-271)
#define TEST_VIRT_ADDR 0xFFFF880000000000ULL
#define TEST_VIRT_ADDR2 0xFFFF880000001000ULL

void test_vmm() {
  print("Running VMM Tests...\n");

  print("Running VMM Tests...\n");

  // ОТЛАДКА PMM: Давайте попробуем выделить один фрейм вручную
  uint64_t test_phys = pmm_alloc_frame();

  // Выведите на экран, что вернул PMM
  print("  [DEBUG] PMM first alloc returned: ");
  printf("%h", test_phys); // Используйте вашу функцию вывода чисел
  print("\n");

  if (test_phys == 0) {
    Panic("PMM IS BROKEN OR NOT INITIALIZED! It returns 0.");
  }
  // ==================================================================
  // ТЕСТ 1: Базовое мапирование и чтение/запись
  // Цель: Проверить, что vmm_map_page создает таблицы, и мы можем
  // писать в выделенную виртуальную память.
  // ==================================================================
  print("  Test 1: Basic Mapping & R/W... ");

  // 1. Выделяем физический фрейм
  u64 phys1 = pmm_alloc_frame();
  ASSERT(phys1 != 0, "PMM failed to alloc frame for Test 1");

  // 2. Мапируем его
  vmm_map_page(&kernel_space, TEST_VIRT_ADDR, phys1,
               PTE_PRESENT | PTE_WRITABLE | PTE_NX);

  // 3. Проверяем, что функция перевода адреса работает
  ASSERT(vmm_virt_to_phys(&kernel_space, TEST_VIRT_ADDR) == phys1,
         "Virt_to_phys mismatch");

  // 4. Пишем данные в виртуальную память и читаем обратно
  volatile u64 *ptr = (volatile u64 *)TEST_VIRT_ADDR;
  *ptr = 0xDEADBEEFCAFEBABE;
  ASSERT(*ptr == 0xDEADBEEFCAFEBABE, "Read/Write data mismatch");

  print("PASSED\n");

  // ==================================================================
  // ТЕСТ 2: Алиасинг (Несколько виртуальных адресов на один физический)
  // Цель: Проверить, что мы можем мапировать разные виртуальные адреса
  // в один и тот же физический фрейм (основа для Shared Memory).
  // ==================================================================
  // ==================================================================
  // ТЕСТ 2: Алиасинг (с отладкой)
  // ==================================================================
  print("  Test 2: Aliasing... \n");

  vmm_map_page(&kernel_space, TEST_VIRT_ADDR2, phys1,
               PTE_PRESENT | PTE_WRITABLE | PTE_NX);

  // ОТЛАДКА: Проверяем, куда ведут адреса ДО записи
  uint64_t p1 = vmm_virt_to_phys(&kernel_space, TEST_VIRT_ADDR);
  uint64_t p2 = vmm_virt_to_phys(&kernel_space, TEST_VIRT_ADDR2);

  // Выводим физические адреса на экран
  print("    [DEBUG] phys1 expected: ");
  printf("%h", phys1); // Вам понадобится функция печати hex-чисел
  print("\n");

  print("    [DEBUG] ADDR1 maps to: ");
  printf("%h", p1);
  print("\n");

  print("    [DEBUG] ADDR2 maps to: ");
  printf("%h", p2);
  print("\n");

  // Проверка 1: Указывают ли они на один и тот же физический фрейм?
  if (p1 != p2) {
    Panic("ALIAS MAPPING MISMATCH! ADDR2 was stolen by Page Fault handler or "
          "VMM failed to map.");
  }

  // Проверка 2: Чтение/запись
  volatile uint64_t *ptr1 = (volatile uint64_t *)TEST_VIRT_ADDR;
  volatile uint64_t *ptr2 = (volatile uint64_t *)TEST_VIRT_ADDR2;

  *ptr1 = 0x123456789ABCDEF0;

  if (*ptr2 != 0x123456789ABCDEF0) {
    print("    [DEBUG] Value read from ADDR2: ");
    printf("%h", *ptr2);
    print("\n");
    Panic("ALIAS READ/WRITE MISMATCH");
  }

  print("  Test 2: PASSED\n");
  // ==================================================================
  // ТЕСТ 3: Анмапирование (Unmapping)
  // Цель: Проверить, что vmm_unmap_page удаляет связь, и
  // virt_to_phys возвращает 0.
  // ==================================================================
  print("  Test 3: Unmapping... ");

  // Предполагается, что у вас есть функция vmm_unmap_page
  vmm_unmap_page(&kernel_space, TEST_VIRT_ADDR2);

  // Физический адрес должен быть 0 (Not Present)
  ASSERT(vmm_virt_to_phys(&kernel_space, TEST_VIRT_ADDR2) == 0,
         "Unmap failed, page still present");

  // Первый адрес должен всё ещё работать
  ASSERT(vmm_virt_to_phys(&kernel_space, TEST_VIRT_ADDR) == phys1,
         "Unmap broke other mapping");

  print("PASSED\n");

  // ==================================================================
  // ТЕСТ 4: Проверка флагов защиты (Read-Only)
  // Цель: Убедиться, что флаги PTE сохраняются правильно.
  // (Мы не можем безопасно проверить Page Fault на запись в ReadOnly
  // без сложной настройки обработчика исключений для тестов, поэтому
  // просто проверим флаги в самой таблице).
  // ==================================================================
  print("  Test 4: Protection Flags... ");

  vmm_unmap_page(&kernel_space, TEST_VIRT_ADDR); // Очищаем старый маппинг

  // Мапируем страницу БЕЗ флага WRITABLE (Только чтение)
  u64 phys2 = pmm_alloc_frame();
  vmm_map_page(&kernel_space, TEST_VIRT_ADDR, phys2,
               PTE_PRESENT | PTE_NX); // Нет PTE_WRITABLE

  // Читаем запись напрямую из таблицы PT, чтобы проверить флаги
  u64 *pml4 = (u64 *)PHYS_TO_VIRT(kernel_space.pml4_phys);
  u64 pml4_e = pml4[PML4_INDEX(TEST_VIRT_ADDR)];
  u64 *pdpt = (u64 *)PHYS_TO_VIRT(pml4_e & PTE_ADDR_MASK);
  u64 pdpt_e = pdpt[PDPT_INDEX(TEST_VIRT_ADDR)];
  u64 *pd = (u64 *)PHYS_TO_VIRT(pdpt_e & PTE_ADDR_MASK);
  u64 pd_e = pd[PD_INDEX(TEST_VIRT_ADDR)];
  u64 *pt = (u64 *)PHYS_TO_VIRT(pd_e & PTE_ADDR_MASK);
  u64 pt_entry = pt[PT_INDEX(TEST_VIRT_ADDR)];

  // Проверяем, что Present = 1, Writable = 0, NX = 1
  ASSERT((pt_entry & PTE_PRESENT) != 0, "Flag Test: Not Present");
  ASSERT((pt_entry & PTE_WRITABLE) == 0, "Flag Test: Should not be Writable");
  ASSERT((pt_entry & PTE_NX) != 0, "Flag Test: Should be NX");

  print("PASSED\n");

  // ==================================================================
  // ТЕСТ 5: Page Fault / Lazy Allocation (Симуляция)
  // Цель: Убедиться, что логика в page_fault_handler работает.
  // ==================================================================
  print("  Test 5: Demand Paging simulation... ");

  // В реальной ситуации это делает код пользователя, но мы можем
  // имитировать это из ядра для проверки.
  // Допустим, адрес TEST_VIRT_ADDR2 сейчас не замаплен.
  ASSERT(vmm_virt_to_phys(&kernel_space, TEST_VIRT_ADDR2) == 0,
         "Pre-condition for demand paging failed");

  // Имитируем то, что сделает обработчик прерывания:
  u64 lazy_phys = pmm_alloc_frame();
  vmm_map_page(&kernel_space, TEST_VIRT_ADDR2, lazy_phys,
               PTE_PRESENT | PTE_WRITABLE | PTE_USER | PTE_NX);

  // Теперь по этому адресу можно писать
  volatile u64 *lazy_ptr = (volatile u64 *)TEST_VIRT_ADDR2;
  *lazy_ptr = 0xAAAAAAAABBBBBBBB;
  ASSERT(*lazy_ptr == 0xAAAAAAAABBBBBBBB, "Demand paging write failed");

  print("PASSED\n");

  print("All VMM Tests Passed!\n");
}
