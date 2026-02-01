#include "pit.h"
#include "io.h"
#include "pic.h"

/* PIT I/O порты */
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND 0x43
#define PIT_BASE_HZ 1193182U /* Входная тактовая частота PIT */

static volatile unsigned long pit_ticks = 0;
extern unsigned int system_ticks; /* определенно в  main.c */

/*
 * Инициализируйте PIT на заданную частоту (Гц)
 * Пример: pit_init(100) → 100 прерываний в секунду (период 10 мс)
 */
void pit_init(unsigned int frequency) {
  unsigned int divisor = PIT_BASE_HZ / frequency;

  /* Канал 0, режим доступа = lobyte/hibyte, режим 2 (генератор скорости) */
  outb(PIT_COMMAND, 0x34);
  outb(PIT_CHANNEL0, divisor & 0xFF);
  outb(PIT_CHANNEL0, divisor >> 8);
}

/*
 * Обработчик IRQ0 (таймер PIT)
 * ------------------------------------------------------------
 * Запускается каждые 10 мс, если частота = 100 Гц.
 * Увеличивает глобальные счетчики тиков и переключает курсор каждые 50 тиков
 * (~0,5 с).
 */
void interrupt_handler_timer(void) {
  pit_ticks++;
  system_ticks++;

  /* Переключать курсор каждые 50 тактов (~0,5 с при частоте 100 Гц) */
  if (pit_ticks % 50 == 0) {
    // toggle_cursor();
  }

  /* ВАЖНО: НЕ печатайте непосредственно здесь — это может привести к зависанию
   или повреждению состояния. Используйте это прерывание только для обновления
   счетчиков или флагов. */

  /* Подтвердите прерывание для PIC, чтобы мог поступить следующий IRQ0 */
  pic_ack(0);
}
