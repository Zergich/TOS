#include <System/interrupts.h>
#include <System/io.h>
#include <System/time.h>
#include <VGA/print.h>
#include <types.h>

TimePit Timepit = {
    .UpTime = UpTime,
    .Sleep = Sleep,
    .PitTimerMiliSecondsUp = 0,
};
// пока временныый вариант. Надо чтоб возвращала значение в виде строки!!!!!
void UpTime() {
  printf("\nSystem already working: %u Seconds\n", Timepit.PitTimerSecondsUp);
}

u64 GetTick() { // без этой функции все идет по пизде потому что компилятор
                // пидорас
  // это защита от кэширования и грубый барьер памяти
  return Timepit.PitTimerMiliSecondsUp;
}
void Sleep(u32 TimeToSleep) {
  u64 StartTime = GetTick(); // Обновление данных и защита от кэширования путем
                             // вызова функции
  // аля барьер памяти

  // Использует разницу это безопасно при переполнении
  while ((int64_t)(GetTick() - StartTime) < (int64_t)TimeToSleep) {
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
