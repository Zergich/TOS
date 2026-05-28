#include <ConsoleIO/print.h>
#include <arch/x86_64/interrupts.h>
#include <arch/x86_64/io.h>
#include <libs/time.h>
#include <types.h>

// пока временныый вариант. Надо чтоб возвращала значение в виде строки!!!!!
void UpTime() {
  printf("System already working: %u Seconds\n", Timepit.PitTimerSecondsUp);
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

TimePit Timepit = {
    .UpTime = UpTime,
    .Sleep = Sleep,
    .PitTimerMiliSecondsUp = 0,
};
