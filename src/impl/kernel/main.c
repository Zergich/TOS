// #include "System/sysinfo.h"
#include <ConsoleIO/shell.h>
#include <System/OSInit.h>
#include <System/interrupts.h>
#include <types.h>

#include <System/time.h>
#include <limine.h>
#include <stddef.h>
#include <string.h>

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

// Маркер конца запросов (обязателен)
__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t
    limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

void kernel_main() {

  if (framebuffer_request.response == NULL ||
      framebuffer_request.response->framebuffer_count < 1) {
  }

  struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

  // Рисуем синий экран (0x0000FF)
  // Используем uintptr_t для работы с 64-битными адресами
  uint32_t *fb_ptr = (uint32_t *)fb->address;
  for (size_t i = 0; i < (fb->pitch / 4) * fb->height; i++) {
    fb_ptr[i] = 0x27F5B0;
  }
  // while (1)
  //   ;

  // WelcomeMessage();
  idt_init();

  // magic байт для проверки интерфейса загрузки и ссылка на структуры для
  // чтения

  // После полной настройки прерываний включаем их
  asm volatile("sti");
  // asm volatile("int $0x0");
  pit_init(1000); // прерывается 1000 раз в секунду

  static string15 pede;
  while (true) {
    Shell();
    // Бесконечный цикл ядра
  }
}
