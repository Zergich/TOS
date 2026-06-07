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

#include <System/Mion/GeometryWindow.h>

// для карты памяти
volatile struct limine_memmap_request *MemMapStructPtr;
volatile struct limine_hhdm_request *HHDMRequest; // для страниц
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
  WelcomeMessage();
  idt_init();
  struct WindowInfo info;
  info.PosY = 10;
  info.PosX = 10;
  info.Height = 5;
  info.Width = 11;
  info.Title = "PEDE123123123123";
  DrawWindow(info);

  asm volatile("sti");

  static string15 pede;
  while (true) {
    Shell();
    // Бесконечный цикл ядра
  }
}
