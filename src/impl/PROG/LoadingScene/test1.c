#include <ConsoleIO/print.h>
#include <System/sysinfo.h>
#include <libs/rand.h>
#include <libs/time.h>
#include <types.h>

void ChaoticTentaclesAnimation(void) {
  int cx = NUM_COLUMS / 2;
  int cy = NUM_ROWS / 2;

#define NUM_TENTACLES 4

  // Стартовые позиции кончиков щупалец
  int tip_x[NUM_TENTACLES] = {cx - 15, cx + 15, cx - 12, cx + 12};
  int tip_y[NUM_TENTACLES] = {cy - 6, cy - 6, cy + 6, cy + 6};

  // Массивы для хранения СТАРЫХ координат кончиков
  int old_tx[NUM_TENTACLES];
  int old_ty[NUM_TENTACLES];

  for (int i = 0; i < NUM_TENTACLES; i++) {
    old_tx[i] = tip_x[i];
    old_ty[i] = tip_y[i];
  }

  while (1) {
    Timepit.Sleep(50);

    // --- ШАГ 1: ПОЛНОЕ СТИРАНИЕ СТАРЫХ ЩУПАЛЕЦ ---
    // Проходим по старым координатам и затираем всю линию целиком
    for (int i = 0; i < NUM_TENTACLES; i++) {
      int steps = 8; // Должно совпасть с количеством шагов при отрисовке!

      for (int s = 1; s <= steps; s++) {
        int old_px = cx + (old_tx[i] - cx) * s / steps;
        int old_py = cy + (old_ty[i] - cy) * s / steps;

        ConsoleSetCarretPos(old_px, old_py);
        print(" ");
      }
      // Стираем старый кончик
      ConsoleSetCarretPos(old_tx[i], old_ty[i]);
      print(" ");
    }

    // --- ШАГ 2: СОХРАНЕНИЕ И ОБНОВЛЕНИЕ КООРДИНАТ ---
    for (int i = 0; i < NUM_TENTACLES; i++) {
      // Запоминаем текущие координаты как старые для следующего кадра
      old_tx[i] = tip_x[i];
      old_ty[i] = tip_y[i];

      // Хаотичное смещение кончика (-1, 0 или +1)
      int dx = (int)((u32)Random.rand() % 3) - 1;
      int dy = (int)((u32)Random.rand() % 3) - 1;

      tip_x[i] += dx;
      tip_y[i] += dy;

      // Ограничиваем зону движения в пределах экрана Limine
      if (tip_x[i] < 5)
        tip_x[i] = 5;
      if (tip_x[i] > NUM_COLUMS - 6)
        tip_x[i] = NUM_COLUMS - 6;
      if (tip_y[i] < 2)
        tip_y[i] = 2;
      if (tip_y[i] > NUM_ROWS - 3)
        tip_y[i] = NUM_ROWS - 3;
    }

    // --- ШАГ 3: РИСУЕМ ЦЕНТРАЛЬНОЕ ЯДРО ---
    for (int oy = -2; oy <= 2; oy++) {
      for (int ox = -5; ox <= Random.rand() % 7; ox++) {
        if ((ox * ox) + 2 * (oy * oy) <= 16) {
          ConsoleSetCarretPos(cx + ox, cy + oy);
          print("O");
        }
      }
    }

    // --- ШАГ 4: РИСУЕМ НОВЫЕ ЗАПОЛНЕННЫЕ ЩУПАЛЬЦА ---
    for (int i = 0; i < NUM_TENTACLES; i++) {
      int steps = 8; // Длина щупальца (количество сегментов линии)

      for (int s = 1; s <= steps; s++) {
        // Вычисляем точки новой линии от центра до нового кончика
        int px = cx + (tip_x[i] - cx) * s / steps;
        int py = cy + (tip_y[i] - cy) * s / steps;

        ConsoleSetCarretPos(px, py);
        print("~");
      }

      // Рисуем кончик щупальца
      ConsoleSetCarretPos(tip_x[i], tip_y[i]);
      print("*");
    }
  }
}
// Глобальный массив для хранения позиции "капли" в каждой колонке.
// Предположим, NUM_COLUMS у тебя около 80.
int drop_y[80];

// Простейший генератор случайных чисел (Linear Congruential Generator),
// если у тебя его еще нет в ядре.
static unsigned long int next = 1;
int rand(void) {
  next = next * 1103515245 + 12345;
  return (unsigned int)(next / 65536) % 32768;
}

void InitSplashMatrix() {
  // Инициализируем капли случайными позициями ВЫШЕ экрана,
  // чтобы они падали не одновременно
  for (int i = 0; i < NUM_COLUMS; i++) {
    drop_y[i] = -(rand() % NUM_ROWS);
  }
}

void DrawMatrixFrame() {
  for (int x = 0; x < NUM_COLUMS; x++) {
    int y = drop_y[x];

    // 1. Закрашиваем хвост (стираем символ высоко над текущей каплей)
    int tail_y = y - 5; // Длина хвоста капли (например, 5 символов)
    if (tail_y >= 0 && tail_y < NUM_ROWS) {
      ConsoleSetCarretPos(x, tail_y);
      // Тут нужно вызвать твою функцию вывода символа, например:
      // ConsolePutChar(' ');
    }

    // 2. Рисуем "голову" капли
    if (y >= 0 && y < NUM_ROWS) {
      ConsoleSetCarretPos(x, y);

      // Выбираем случайный ASCII символ от 33 ('!') до 126 ('~')
      char random_char = 33 + (rand() % 94);

      // Устанавливаем зеленый цвет и печатаем символ
      // SetColor(VGA_COLOR_LIGHT_GREEN);
      // ConsolePutChar(random_char);
    }

    // 3. Двигаем каплю вниз
    drop_y[x]++;

    // 4. Если капля улетела за нижний край с учетом хвоста — сбрасываем ее
    // наверх
    if (drop_y[x] - 5 > NUM_ROWS) {
      drop_y[x] = 0; // или снова -(rand() % 10)
    }
  }
}
void Start() {
  ChaoticTentaclesAnimation();
  // for (int i = 0; i < 30; i++) {
  //   u16 random_x = (u16)((u32)Random.rand() % (u32)NUM_COLUMS);
  //   u16 random_y = (u16)((u32)Random.rand() % (u32)NUM_ROWS);
  //
  //   ConsoleSetCarretPos(random_x, random_y);
  //   print("p");
  // }
}
