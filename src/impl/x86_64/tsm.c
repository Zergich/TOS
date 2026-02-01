//
// #include "io.h"
// #include "pit.h"
// #include <print.h>
// static uint32_t hbt = 0;
// static double best_dv = 99.9;
//
// void tsm_optimize() {
//   double dv = 21.4 - (hbt * 0.01);
//   if (dv < best_dv) {
//     best_dv = dv;
//     // printf("hbt:%lu | Δv:%.2f km/s | Earth->Mars->Jupiter->Earth\n", hbt /
//     // 100,
//     //        best_dv);
//   }
// }
//
// void pit_handler() {
//   hbt++;
//   if (hbt % 100 == 0)
//     tsm_optimize();
//   pic_eoi(0);
// }
//
// void tsm_init() {
//   pit_set_frequency(100);
//   register_irq(0, pit_handler);
//   print("iTSP LOADED. AMOEBA IS BREATHING.\n");
// }
