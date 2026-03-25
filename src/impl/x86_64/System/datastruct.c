#include <VGA/print.h>
#include <datastruct.h>
#include <types.h>

/*
 *        НАЧАЛО
 * =====================
 * \  Куруговой буффер  \
 * =====================
 */

u8 RoundBuffChar[8192];

RoundBufferObgect RoundBuff = {
    .put = PutIntoBuffer,
    .get = GetgFromBuffer,
    .buffer = {0},
};
volatile int head = 0;
volatile int tail = 0;

enum ExitCode { BUFFER_ROUNDED = 1, ERROR = -1, OK = 0 };

int PutIntoBuffer(u8 character) {
  int ExitCode = OK;
  RoundBuff.buffer[head] = character;
  int next_pos =
      (head + 1) % 8192; // перезаписывает самые старые участки в буфере
  if (next_pos == head) {
    // Буфер полон, сдвигаем хвост (потеряем самый старый символ)
    head = (head + 1) % 8192;
  }

  head = next_pos;

  return ExitCode;
}

int GetgFromBuffer(u8 *linkchar) {

  if (head == tail) {
    return ERROR; // Буффер пуст
  }

  *linkchar = RoundBuff.buffer[tail];
  // printf("%u", RoundBuff.buffer[tail]);
  tail = (tail + 1) % 8192; // та же самая перезапись

  return OK;
}

/*
 *        КОНЕЦ
 * =====================
 * \  Куруговой буффер  \
 * =====================
 */
