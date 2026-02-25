#include <console.h>
#include <datastruct.h>
#include <print.h>
#include <types.h>

extern RoundBufferObgect RoundBuff;

ConsoleInput Console = {.ReadLine = ConsoleRead, .ReadKey = ReadKey

};

int ConsoleRead(char *string) { // мб спипать спец коды и отсавлять только
                                // аски соответственно.
  // это понадобится для чтении клавиши, ведь при текущей
  // реализации читает только ascii без спец кодовх
  uint16_t i = 0;
  char c = 0;
  int max_len = 100;
  // Цикл продолжается, пока не нажмут Enter или не закончится место
  while (i < max_len - 1) {
    // 1. Пытаемся получить символ из буфера
    int code = RoundBuff.get(&c);
    if (code != 0) {
      // printf("%i", code);
      // Буфер пуст (клавишу еще не нажали)
      // Вместо бесконечного нагрузочного цикла ставим процессор на паузу
      // (hlt). Он проснется только когда придет прерывание (нажатие
      // клавиши).
      asm volatile("hlt");
      continue;
    }

    // 2. Обработка клавиши ENTER
    // Клавиша Enter обычно посылает код 0x0D ('\r'), иногда 0x0A ('\n')
    if (c == '\r' || c == '\n') {
      PrintChar('\n'); // Перевести строку на экране для красоты
      break;           // Выход из цикла
    }

    // 3. Обработка клавиши BACKSPACE (удаление)
    // Код Backspace - 0x08
    if (c == 0x08) {
      if (i > 0) {
        i--;             // Удаляем символ из буфера
        PrintChar('\b'); // Сдвигаем курсор назад (визуально)
        // Здесь можно реализовать очистку символа пробелом и возврат назад:
        // Console_PutChar(' '); Console_PutChar('\b');
      }
      continue; // Не сохраняем сам символ Backspace в строку
    }

    // 4. Обычный символ
    // (Опционально) Фильтр управляющих символов, если c < 32 (кроме \t)

    string[i++] = c; // Сохраняем в буфер
    PrintChar(c);    // Выводим на экран (эхо)
  }

  // Завершаем строку нулем
  string[i] = '\0';

  return string;
}
char ReadKey() {
  char GetChar;
  RoundBuff.get(GetChar);
  if (GetChar) {
    return GetChar;
  }
  return -1;
}
