#include <datastruct.h>
#include <types.h>

/*
 *        НАЧАЛО
 * =====================
 * \  Куруговой буффер  \
 * =====================
 */

char RoundBuffChar[8192];

RoundBufferObgect RoundBuff = {
    .position = 0, .put = PutIntoBuffer, .get = GetgFromBuffer, .buffer = {0}

};
volatile int head = 0;
volatile int tail = 0;

enum ExitCode { BUFFER_ROUNDED = 1, ERROR = -1, OK = 0 };

int PutIntoBuffer(char character) {
  int ExitCode = OK;
  RoundBuff.buffer[head] = character;

  // Двигаем позицию записи вперед с цикличностью

  int next_pos = (head + 1) % 8192;

  // 3. Обработка переполнения (если мы догнали хвост чтения)
  if (next_pos == head) {
    // Буфер полон, сдвигаем хвост (потеряем самый старый символ)
    head = (head + 1) % 8192;
  }

  head = next_pos;

  return ExitCode; // ok
}

int GetgFromBuffer(char *linkchar) {

  if (head == tail) {
    return ERROR; // Ошибка: пусто
  }

  *linkchar = RoundBuff.buffer[tail];
  tail = (tail + 1) % 8192;
  return OK;
}

/*
 *        КОНЕЦ
 * =====================
 * \  Куруговой буффер  \
 * =====================
 */
