#include <ConsoleIO/print.h>
#include <libs/rand.h>
int RandInit(int seed);
int Rand();
RandomStr Random = {
    .rand = Rand,
    .init = RandInit,
};

volatile int Seed = 12341414; // default
int RandInit(int seed) {
  if (seed == 0) // при xor рандоме сид не долэин быть равен 0
    return -1;
  Seed = seed;
  // print(seed);
  // print("\n");
  return 0;
}

int Rand() {
  int state = Seed;
  state ^= state << 18;
  state ^= state << 6;
  state ^= state >> 16;
  return Seed = state;
}
