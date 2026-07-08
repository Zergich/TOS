#include <ConsoleIO/print.h>
#include <libs/format.h>
#include <stddef.h>
static const char *mem_units[] = {"B", "KiB", "MiB", "GiB", "TiB"};

// Функция сразу печатает красивый размер памяти в консоль
void print_memory_size(size_t bytes) {
  size_t size = bytes;
  size_t remainder = 0;
  int unit_index = 0;

  // Делим на 1024, пока число большое
  while (size >= 1024 && unit_index < 4) {
    remainder = size % 1024;
    size /= 1024;
    unit_index++;
  }

  if (unit_index == 0) {
    // Если байты — выводим как обычно
    printf("%u %s", size, mem_units[unit_index]);
  } else {
    // Хитрый расчет: переводим остаток в десятые доли (0-9) с округлением
    size_t tenths = (remainder * 10) / 1024;

    // Выводим целую часть, точку, дробную часть и единицу измерения
    printf("%u.%u %s", size, tenths, mem_units[unit_index]);
  }
}
