// #include "System/sysinfo.h"
#include "System/MemoryManager/PMM.h"
#include "System/sysinfo.h"
#include <ConsoleIO/shell.h>
#include <System/OSInit.h>
#include <arch/x86_64/interrupts.h>
#include <stdint.h>
#include <types.h>

#include <ConsoleIO/graphics.h>
#include <ConsoleIO/print.h>
#include <libs/string.h>
#include <libs/time.h>
#include <limine.h>
#include <stddef.h>

extern Pixeling PixelGrapchics;
extern TimePit Timepit;
extern StringStruct string;

// Маркер начала запросов (обязателен)
__attribute__((used,
               section(".limine_requests_start"))) static volatile uint64_t
    limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

// САМ ЗАПРОС ФРЕЙМБУФЕРА
__attribute__((
    used,
    section(
        ".limine_requests"))) static volatile struct limine_framebuffer_request
    framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST_ID, .revision = 0};

__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_memmap_request
    memmap_request = {.id = LIMINE_MEMMAP_REQUEST_ID, .revision = 0};

__attribute__((
    used,
    section(".limine_requests"))) static volatile struct limine_hhdm_request
    hhdm_request = {.id = LIMINE_HHDM_REQUEST_ID, .revision = 0};
// Маркер конца запросов (обязателен)
__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t
    limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// для карты памяти
volatile struct limine_memmap_request *MemMapStructPtr;
volatile struct limine_hhdm_request *HHDMRequest; // для страниц

void test_pmm_init() {
  uint64_t free_pages = pmm_get_free_pages();

  // Просто выводим количество свободной памяти
  printf("[PMM TEST] Free pages after init: %u (%u MB)\n", free_pages,
         (free_pages * 4096) / (1024 * 1024));

  // Если свободных страниц 0 — это фатально
  if (free_pages == 0) {
    printf("[PMM TEST] FATAL: No free memory!\n");
    while (1)
      ; // Останавливаемся
  }
}
void test_pmm_alloc_free(uintptr_t hhdm_offset) {
  printf("[PMM TEST] Allocating 1 page...\n");
  uintptr_t p1 = pmm_alloc_page();

  if (p1 == 0) {
    printf("[PMM TEST] FAILED: Allocation returned 0!\n");
    return;
  }
  printf("[PMM TEST] Allocated physical address: 0x%llx\n", p1);

  // САМАЯ ВАЖНАЯ ПРОВЕРКА: Можем ли мы реально писать в эту память?
  // Преобразуем физический адрес в виртуальный через HHDM
  uint64_t *virt = (uint64_t *)(p1 + hhdm_offset);
  *virt = 0xDEADBEEFCAFEBABE; // Пишем магическое число

  // Читаем обратно. Если PMM выдал битмап вместо памяти, тут будет мусор или
  // падение
  if (*virt != 0xDEADBEEFCAFEBABE) {
    printf("[PMM TEST] FAILED: Cannot write/read to allocated memory!\n");
  } else {
    printf("[PMM TEST] Memory write/read OK!\n");
  }

  // Проверяем, что счетчик уменьшился
  uint64_t free_after_alloc = pmm_get_free_pages();

  printf("[PMM TEST] Freeing page 0x%llx...\n", p1);
  pmm_free_page(p1);

  // Проверяем, что счетчик вернулся в норму
  uint64_t free_after_free = pmm_get_free_pages();

  if (free_after_free != free_after_alloc + 1) {
    printf("[PMM TEST] FAILED: Free pages counter mismatch after free!\n");
  } else {
    printf("[PMM TEST] Free/Alloc counter OK!\n");
  }
}
void test_pmm_double_free() {
  uintptr_t p = pmm_alloc_page();
  uint64_t free_before = pmm_get_free_pages();

  printf("[PMM TEST] Testing double free protection...\n");
  pmm_free_page(p); // Первое освобождение (нормально)
  pmm_free_page(p); // Второе освобождение (ОШИБКА, должно быть проигнорировано)

  uint64_t free_after = pmm_get_free_pages();

  // Счетчик должен был увеличиться только на 1
  if (free_after != free_before + 1) {
    printf("[PMM TEST] FAILED: Double free corrupted the free counter! (%u vs "
           "%u)\n",
           free_before + 1, free_after);
  } else {
    printf("[PMM TEST] Double free protection works!\n");
  }
}

void test_pmm_exhaustion(uintptr_t hhdm_offset) {
  printf("[PMM TEST] Starting exhaustion test...\n");
  uint64_t initial_free = pmm_get_free_pages();
  uint64_t allocated_count = 0;

  // Выделяем страницы, пока они не закончатся
  while (1) {
    uintptr_t p = pmm_alloc_page();
    if (p == 0) {
      break; // Память закончилась!
    }

    // Обязательно пишем в каждую страницу! Если PMM выдал одну и ту же страницу
    // дважды, мы затрем данные, и ядро упадет. Это проверка на дублирование
    // адресов.
    uint64_t *virt = (uint64_t *)(p + hhdm_offset);
    *virt = allocated_count;

    allocated_count++;
  }

  printf("[PMM TEST] Exhausted memory! Allocated %u pages.\n", allocated_count);

  if (pmm_get_free_pages() != 0) {
    printf("[PMM TEST] FAILED: Free pages is not 0 after exhaustion!\n");
  }

  // Теперь освобождаем всё, что выделили.
  // Проблема: мы не сохранили адреса. Как освободить?
  // В реальном ядре адреса хранятся в структурах VMM.
  // Для этого теста мы можем просто заново проинициализировать PMM,
  // либо, если хотите, можно модифицировать тест, чтобы он выделил, скажем,
  // только 1000 страниц и освободил их.

  // Для простоты сейчас просто сбросим PMM:
  pmm_init();
  uint64_t free_after_reset = pmm_get_free_pages();

  if (free_after_reset != initial_free) {
    printf("[PMM TEST] FAILED: Memory leak detected after reset! (%u vs %u)\n",
           initial_free, free_after_reset);
  } else {
    printf("[PMM TEST] Memory fully recovered after exhaustion test.\n");
  }
}
void kernel_main() {

  if (framebuffer_request.response == NULL ||
      framebuffer_request.response->framebuffer_count < 1) {
  }

  struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

  uint32_t *fb_ptr = (uint32_t *)fb->address;
  PixelGrapchics.Init(fb_ptr);
  InitConstantGraphics(fb->pitch / 4, fb->width, fb->height);

  // оберка передачи данных в функцию геморойней с графикой другое дело
  // передача указателся на структуру карты памяти
  MemMapStructPtr = &memmap_request;
  HHDMRequest = &hhdm_request;

  idt_init();

  // После полной настройки прерываний включаем их
  asm volatile("sti");

  WelcomeMessage();
  pmm_init();
  test_pmm_init();
  uptr pede213;
  test_pmm_alloc_free(pede213);
  test_pmm_double_free();
  test_pmm_exhaustion(pede213);

  static string15 pede;
  while (true) {
    Shell();
    // Бесконечный цикл ядра
  }
}
