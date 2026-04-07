#include <System/Array.h>
#include <System/MemoryUtils.h>

int IndexInsertC(char *arr, int *size, int max_size, int index,
                 char value) { // C - значит что для char
  // size + 1: нам нужно место для нового символа
  // size + 2: нам нужно место для нуль-терминатора в конце (\0)
  if (*size + 1 >= max_size || index < 0 || index > *size) {
    return -1;
  }

  // Сдвигаем ТОЛЬКО длину строки (size) минус текущий индекс.
  // Мы НЕ добавляем +1 к размеру, чтобы не копировать мусор из arr[size].
  memmove(&arr[index + 1], &arr[index], (*size - index) * sizeof(char));

  arr[index] = value;

  // Увеличить размер
  (*size)++;

  arr[*size] = '\0';

  return 0;
}

// Удаление элемента по индексу
int IndexDeleteC(char *arr, int *size, int index) {
  if (index < 0 || index >= *size) {
    return -1;
  }

  // Сдвигаем элементы слева
  memmove(&arr[index], &arr[index + 1], (*size - index - 1) * sizeof(char));
  (*size)--;
  arr[*size] = '\0';

  return 0;
}
