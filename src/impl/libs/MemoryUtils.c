#include <libs/MemoryUtils.h>
#include <stddef.h>

void *memcpy(void *dst, const void *src, size_t size) {
  unsigned char *d = (unsigned char *)dst;
  const unsigned char *s = (const unsigned char *)src;

  for (size_t i = 0; i < size; i++) {
    d[i] = s[i];
  }

  return dst; // Стандартный memcpy возвращает указатель на приемник
}

void memset(void *dst, unsigned char value, int count) {
  for (int i = 0; i < count; i++) {
    ((unsigned char *)dst)[i] = value;
  }
}
int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;

  for (size_t i = 0; i < n; i++) {
    if (p1[i] != p2[i]) {
      return p1[i] - p2[i]; // Возвращаем разницу между байтами
    }
  }

  return 0; // Все n байт совпали
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
