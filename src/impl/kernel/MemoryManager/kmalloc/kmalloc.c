#include <System/MemoryManager/PMM.h>
#include <System/MemoryManager/VMM.h>
#include <ConsoleIO/print.h>
#include <System/rsod.h>
#include <stddef.h>

// Подключаем заголовки TLSF
#include <System/MemoryManager/kmalloc/tlsf.h>
#include <System/MemoryManager/kmalloc/kmalloc.h>

static tlsf_t main_tlsf_instance = NULL;

#define KERNEL_HEAP_START 0xFFFF900000000000ULL
#define KERNEL_HEAP_SIZE  (16 * 1024 * 1024) // 16 МБ
#define PAGE_SIZE         4096

// Стандартные флаги без HUGE бита
#define x86_PTE_PRESENT  (1ULL << 0)
#define x86_PTE_WRITABLE (1ULL << 1)

void kmalloc_init()
{

    uintptr_t pool_start = KERNEL_HEAP_START;
    size_t pool_size = KERNEL_HEAP_SIZE;

    extern address_space_t kernel_space;

    for (uintptr_t offset = 0; offset < KERNEL_HEAP_SIZE; offset += PAGE_SIZE) {
        // Честно просим у PMM физический фрейм для каждой страницы кучи
        uintptr_t phys = pmm_alloc_frame();
        
        if (phys == 0) {
            Panic("[KMALLOC INIT] Out of physical memory while initializing kernel heap!");
        }
        
        // Мапируем виртуальный адрес кучи на честный физический фрейм
        vmm_map_page(&kernel_space, KERNEL_HEAP_START + offset, phys, 
                     x86_PTE_PRESENT | x86_PTE_WRITABLE);
    }
    

    // Инициализируем TLSF на полностью готовом и размеченном регионе памяти
    main_tlsf_instance = tlsf_create_with_pool((void*)pool_start, pool_size);
    
    if (main_tlsf_instance == NULL) {
        Panic("Kmalloc initialization failed: tlsf_create_with_pool returned NULL!");
    }

}

// Стандартная аллокация
void* kmalloc(size_t size) {
    if (size == 0 || main_tlsf_instance == NULL) {
        return NULL;
    }
    
    // Передаем инстанс аллокатора ПЕРВЫМ аргументом, а размер ВТОРЫМ
    return tlsf_malloc(main_tlsf_instance, size);
}

// Освобождение памяти
void kfree(void* ptr) {
    if (ptr == NULL || main_tlsf_instance == NULL) {
        return;
    }
    
    // Передаем инстанс аллокатора ПЕРВЫМ аргументом, а указатель ВТОРЫМ
    tlsf_free(main_tlsf_instance, ptr);
}

typedef struct {
    size_t used_bytes;
    size_t free_bytes;
} my_mem_stats_t;

static void my_stats_walker(void* ptr, size_t size, int used, void* user) {
    my_mem_stats_t* stats = (my_mem_stats_t*)user;
    
    if (used) {
        stats->used_bytes += size;
    } else {
        stats->free_bytes += size;
    }
}

tlsf_stats_t MemHeapInfo()
{
 tlsf_stats_t KernelHeapStatus = {0,0};
  pool_t pool = tlsf_get_pool(main_tlsf_instance);

    tlsf_walk_pool(pool, my_stats_walker, (void*)&KernelHeapStatus);

    return KernelHeapStatus;
}

void test_kernel_heap() {
    print("Running Kernel Heap Stress Test... \n");

    // Тест 1: Базовое выделение разных размеров
    int* ptr1 = (int*)kmalloc(sizeof(int) * 100);
    char* ptr2 = (char*)kmalloc(250);
    uint64_t* ptr3 = (uint64_t*)kmalloc(4096); // Ровно страница

    if (!ptr1 || !ptr2 || !ptr3) Panic("Heap Test 1 Failed: Allocation returned NULL");

    // Пишем данные, проверяем, что нет Page Fault
    ptr1[0] = 12345;
    ptr2[0] = 'A';
    ptr3[0] = 0xDEADBEEF;

    // Тест 2: Освобождение и переиспользование (Устойчивость к фрагментации)
    kfree(ptr2); 
    
    // TLSF должен отдать тот же самый адрес (или рядом), так как блок освободился
    char* ptr2_new = (char*)kmalloc(250);
    if (!ptr2_new) Panic("Heap Test 2 Failed: Re-allocation returned NULL");
    
    ptr2_new[0] = 'B';

    // Тест 3: Массовая аллокация (проверяем стабильность пула)
    void* array[500];
    for (int i = 0; i < 500; i++) {
        array[i] = kmalloc(32); // Выделяем много мелких блоков
        if (!array[i]) Panic("Heap Test 3 Failed: Mass allocation choked");
    }

    // Чистим за собой абсолютно всё
    for (int i = 0; i < 500; i++) {
        kfree(array[i]);
    }
    kfree(ptr1);
    kfree(ptr2_new);
    kfree(ptr3);

    print("Kernel Heap Stress Test: ALL PASSED!\n");
}
