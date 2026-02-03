#include "pit.h" // For PIT initialization and timer interrupt handling
#include <OSInit.h>
#include <bool.h>
#include <interrupts.h>
#include <print.h>
#include <shell.h>
unsigned int system_ticks = 0;

void kernel_main() {
  ConsoleClear();
  WelcomeMessage();

  // 2. Инициализируйте таблицу дескрипторов прерываний (IDT).
  init_idt();

  // 3. Инициализируйте программируемый контроллер прерываний (РИС.)
  pic_init();

  // 4. Инициализируйте таймер с программируемым интервалом (PIT)
  //
  //
  // Установите PIT для генерации 100 прерываний в секунду (100 Гц).
  // Это управляет синхронизацией системы (мигание курсора, счетчик времени
  // безотказной работы и т.д.).
  pit_init(100);

  init_shell();
  // int a = 2 / 0;
  // 5. Инициализируйте оболочку (текстовый пользовательский интерфейс)

  // 6. Включите прерывания процессора глобально
  // __asm__ volatile("sti"); // hlt дрочится потому что нет шела (того кто
  // примет ввод) по этому система ребутается

  // 7. Idle loop (halt until next interrupt)
  // Центральный процессор будет находиться в спящем режиме до тех пор, пока
  // его не прервет (например, PIT или клавиатура)
  //. Обработчик PIT увеличивает значение system_ticks.
  while (true) {
    __asm__ volatile("hlt");
  }
}
