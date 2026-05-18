#include <libs/MemoryUtils.h>

void memcpy(void *dst, void *src, int size) {
  for (int i = 0; i < size; i++) {
    char *src_byte = src + i;
    char *dst_byte = dst + i;
    *dst_byte = *src_byte;
  }
}

void memset(void *dst, unsigned char value, int count) {
  for (int i = 0; i < count; i++) {
    ((unsigned char *)dst)[i] = value;
  }
}

/**
 * Копирует n байт из src в dest. Области памяти могут перекрываться.
 * Возвращает указатель на dest.
 */
void *memmove(void *dest, const void *src, unsigned int n) {
  // Приводим указатели к байтовым типам для побайтового копирования
  unsigned char *d = (unsigned char *)dest;
  const unsigned char *s = (const unsigned char *)src;

  // Если копировать нечего или адреса совпадают
  if (n == 0 || dest == src) {
    return dest;
  }

  // Проверяем перекрытие.
  // Если адрес назначения больше адреса источника, значит мы можем
  // затереть еще не прочитанные данные при прямом копировании.
  // Поэтому копируем с конца.
  if (d > s) {
    // Копирование назад (с конца массива)
    d += n;
    s += n;
    while (n--) {
      *--d = *--s;
    }
  } else {
    // Копирование вперед (с начала массива)
    // Это безопасно, так как мы уходим вперед и не затираем то, что еще не
    // прочитали
    while (n--) {
      *d++ = *s++;
    }
  }

  return dest;
}
