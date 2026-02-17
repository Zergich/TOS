#include <OSInit.h>
#include <bool.h>
#include <interrupts.h>
#include <print.h>
void kernel_main() {
  ConsoleClear();
  WelcomeMessage();

  idt_init();
  // test_interrupt();
  // int a = 2 / 0;

  // Для проверки — вызов деления на ноль
  // Это вызовет обработчик
  init_shell();

  // ------------------------------------------------------------
  // 6. Enable CPU interrupts globally
  // ------------------------------------------------------------
  __asm__ volatile("sti");

  // ------------------------------------------------------------
  // 7. Idle loop (halt until next interrupt)
  // ------------------------------------------------------------
  // The CPU will sleep until an interrupt (like the PIT or keyboard)
  // wakes it. The PIT handler increments system_ticks.
  while (1) {
    __asm__ volatile("hlt");
  }
}
