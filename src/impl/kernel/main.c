// #include "System/sysinfo.h"
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

#include <System/MemoryManager/PMM.h>
#include <System/MemoryManager/VMM.h>

#include <System/MemoryManager/kmalloc/kmalloc.h>

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

void enable_sse() {
  uint64_t cr0, cr4;

  // Читаем CR0, настраиваем биты, пишем обратно
  asm volatile("mov %%cr0, %0" : "=r"(cr0));
  cr0 &= ~(1 << 2); // Сброс EM
  cr0 |= (1 << 1);  // Установка MP
  asm volatile("mov %0, %%cr0" ::"r"(cr0));

  // Читаем CR4, включаем биты 9 и 10 для SSE
  asm volatile("mov %%cr4, %0" : "=r"(cr4));
  cr4 |= (1 << 9) | (1 << 10);
  asm volatile("mov %0, %%cr4" ::"r"(cr4));
}

void kernel_main() {
  enable_sse();
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
  WelcomeMessage();
  idt_init();
  pmm_init();
  asm volatile("sti");

  vmm_init();
  kmalloc_init();
  cpu_init(&CPUInfo);

  static string15 pede;
  while (true) {
    Shell();
    // Бесконечный цикл ядра
  }
}
