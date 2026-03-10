#include <print.h>
#include <time.h>
#include <types.h>

TimePit Timepit = {
    .UpTime = UpTime,
    .Sleep = Sleep,
    .TickEvent = PitTickEvent,
    .PitTimerSecondsUp = 0,
};
// пока временныый вариант. Надо чтоб возвращала значение в виде строки!!!!!
void UpTime() {
  printf("\nIt's already working: %u Seconds\n", Timepit.PitTimerSecondsUp);
}

void Sleep(u32 TimeToSleep) {}

int PitTickEvent() {}
