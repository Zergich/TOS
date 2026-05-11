// #include "System/sysinfo.h"
#include "System/sysinfo.h"
#include <ConsoleIO/shell.h>
#include <System/OSInit.h>
#include <System/interrupts.h>
#include <stdint.h>
#include <types.h>

#include <ConsoleIO/graphics.h>
#include <ConsoleIO/print.h>
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

// для карты памяти
volatile struct limine_memmap_request *MemMapStructPtr;
void kernel_main() {

  if (framebuffer_request.response == NULL ||
      framebuffer_request.response->framebuffer_count < 1) {
  }

  struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

  uint32_t *fb_ptr = (uint32_t *)fb->address;
  PixelGrapchics.Init(fb_ptr);
  InitConstantGraphics(fb->pitch / 4, fb->width, fb->height);

  // передача указателся на структуру карты памяти
  MemMapStructPtr = &memmap_request;

  // for (size_t i = 0; i < WidthDisplay * HeightDisplay; i++) {
  //   fb_ptr[i] = 0x120F2E;
  // }
  // // PixelGrapchics.Draw(0, 3000 * 30, 0x784F2E);
  // PixelGrapchics.DrawChar(0, 0, 'A', 0x27F53F, 0x000000);
  // PixelGrapchics.DrawChar(9, 0, 'y', 0x27F53F, 0x000000);
  // PixelGrapchics.DrawChar(0, 17, 'p', 0x27F53F, 0x000000);

  WelcomeMessage();
  idt_init();
  // printf("%u|%u|", fb->width, fb->width);

  // После полной настройки прерываний включаем их
  asm volatile("sti");
  pit_init(1000); // прерывается 1000 раз в секунду

  static string15 pede;
  while (true) {
    Shell();
    // Бесконечный цикл ядра
  }
}
