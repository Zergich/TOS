#include <interrupts.h>
#include <io.h>
#include <print.h>
#include <time.h>
#include <types.h>

TimePit Timepit = {
    .UpTime = UpTime,
    .Sleep = Sleep,
    .PitTimerMiliSecondsUp = 0,
};
// пока временныый вариант. Надо чтоб возвращала значение в виде строки!!!!!
void UpTime() {
  printf("\nIt's already working: %u Seconds\n", Timepit.PitTimerSecondsUp);
}

void Sleep(u32 TimeToSleep) {
  uint64_t start_time = Timepit.PitTimerMiliSecondsUp;

  while ((int64_t)(Timepit.PitTimerMiliSecondsUp - start_time) <
         (int64_t)TimeToSleep) {
    // hlt заставит процессор спать и потреблять 0% ресурсов,
    // пока не придет прерывание таймера, чтобы обновить условие цикла.
    asm volatile("hlt");
  }
}

void pit_init(int hz) {
  int divisor = 1193180 / hz; // Рассчитываем делитель
                              // выбор канала (нулевой канал)
  outb(PIT_CMD, 0x36);

  // отправка делителей сначала младший потом старший
  uint8_t low = (uint8_t)(divisor & 0xFF);
  uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

  outb(PIT_CH0, low);
  outb(PIT_CH0, high);
}
