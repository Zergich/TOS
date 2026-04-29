// #include "System/sysinfo.h"
#include "System/sysinfo.h"
#include <ConsoleIO/shell.h>
#include <System/OSInit.h>
#include <System/interrupts.h>
#include <stdint.h>
#include <types.h>

#include <ConsoleIO/graphics.h>
#include <System/time.h>
#include <limine.h>
#include <stddef.h>
#include <string.h>

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

// Маркер конца запросов (обязателен)
__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t
    limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

void ReturnMem() {
  uint64_t total_usable = 0;
  uint64_t total_reserved = 0;

  for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
    struct limine_memmap_entry *entry = memmap_request.response->entries[i];

    if (entry->type == LIMINE_MEMMAP_USABLE) {
      total_usable += entry->length;
    } else {
      total_reserved += entry->length;
    }
  }
}
void DrawTest(u64 x, u64 y, u32 color, u32 *pede1) {
  for (int cy = 0; cy < 16; cy++) {
    for (int cx = 0; cx < 8; cx++) {
      pede1[(y + cy) * WidthDisplay + (x + cx)] = color;
    }
  }
}

void kernel_main() {

  if (framebuffer_request.response == NULL ||
      framebuffer_request.response->framebuffer_count < 1) {
  }

  struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

  uint32_t *fb_ptr = (uint32_t *)fb->address;
  PixelGrapchics.Init(fb_ptr);
  InitConstantGraphics(fb->pitch / 4, fb->height);

  for (size_t i = 0; i < WidthDisplay * HiegthDisplay; i++) {
    fb_ptr[i] = 0x120F2E;
  }
  // PixelGrapchics.Draw(0, 3000 * 30, 0x784F2E);
  PixelGrapchics.DrawChar(100, 1, '1', 0x27F53F);
  DrawTest(100, 1, 0x27F53F, fb_ptr);
  WelcomeMessage();
  idt_init();

  // После полной настройки прерываний включаем их
  asm volatile("sti");
  pit_init(1000); // прерывается 1000 раз в секунду

  static string15 pede;
  while (true) {
    Shell();
    // Бесконечный цикл ядра
  }
}
