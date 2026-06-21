#include <ConsoleIO/print.h>
#include <arch/x86_64/interrupts.h>
#include <arch/x86_64/io.h>
#include <libs/time.h>
#include <types.h>


#include <arch/x86_64/io.h>
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



// Функция чтения одного регистра RTC
u8 read_rtc_register(u8 reg) {
  outb(0x70, reg);
  return inb(0x71);
}

// Проверка: идет ли сейчас обновление данных в RTC?
// Если читать во время обновления, можно получить битые данные.
bool is_rtc_updating() {
  outb(0x70, 0x0A); // Регистр статуса A
  return (inb(0x71) & 0x80);
}

// Главная функция получения времени
struct DateTime GetTimeRTS() {
  struct DateTime datetime;

  // Ждем, пока RTC завершит внутреннее обновление
  while (is_rtc_updating())
    ;

  datetime.second = read_rtc_register(0x00);
  datetime.minute = read_rtc_register(0x02);
  datetime.hour = read_rtc_register(0x04);
  datetime.day = read_rtc_register(0x07);
  datetime.month = read_rtc_register(0x08);
  u32 raw_year = read_rtc_register(0x09);

  // === КРИТИЧЕСКИЙ НЮАНС 1: Декодирование BCD ===
  // Большинство чипов RTC по умолчанию возвращают данные в формате BCD
  // (Binary-Coded Decimal). Это значит, что число 26 в байте будет записано как
  // 0x26 (в шестнадцатеричном виде). Нам нужно перевести это в обычные числа.
  u8 status_b = read_rtc_register(0x0B); // Регистр статуса B

  if (!(status_b & 0x04)) { // Если бит 2 равен 0, значит формат BCD
    datetime.second = (datetime.second & 0x0F) + ((datetime.second / 16) * 10);
    datetime.minute = (datetime.minute & 0x0F) + ((datetime.minute / 16) * 10);
    datetime.hour =
        (datetime.hour & 0x0F) + (((datetime.hour & 0x70) / 16) * 10) |
        (datetime.hour & 0x80);
    datetime.day = (datetime.day & 0x0F) + ((datetime.day / 16) * 10);
    datetime.month = (datetime.month & 0x0F) + ((datetime.month / 16) * 10);
    raw_year = (raw_year & 0x0F) + ((raw_year / 16) * 10);
  }

  // === КРИТИЧЕСКИЙ НЮАНС 2: 12/24 часовой формат ===
  // Если RTC настроен на 12-часовой формат, старший бит часа равен 1 для
  // времени PM (после полудня)
  if (!(status_b & 0x02) && (datetime.hour & 0x80)) {
    datetime.hour = ((datetime.hour & 0x7F) + 12) % 24;
  }

  // Достраиваем полный год (RTC хранит только две цифры)
  datetime.year = 2000 + raw_year;

  return datetime;
}
