#include <datastruct.h>
#include <types.h>

/*
 *        НАЧАЛО
 * =====================
 * \  Куруговой буффер  \
 * =====================
 */

char RoundBuffChar[8192];

RoundBufferObgect RoundBuff = {.position = 0,
                               .put = PutIntoBuffer,
                               .get = GetgFromBuffer,
                               .buffer = RoundBuffChar

};

int PutIntoBuffer(char character) {
  RoundBuff.buffer[RoundBuff.position] = character;
  RoundBuff.position++;
  return 0;
}

int GetgFromBuffer(char *linkchar) {

  // for (u16 i = 0; i < RoundBuff.position; i++) {
  //   ch = RoundBuff.buffer[i];
  // }
  linkchar = RoundBuff.buffer[RoundBuff.position];
  return 0;
}

/*
 *        КОНЕЦ
 * =====================
 * \  Куруговой буффер  \
 * =====================
 */
