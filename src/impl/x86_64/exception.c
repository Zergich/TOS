#include "interrupts.h"
#include <print.h>

// Простой strlen для ядра
int strlen(const char *str) {
  int len = 0;
  while (str[len])
    len++;
  return len;
}

void exception_handler(unsigned int interrupt_no, unsigned int error_code) {
  // Преобразовать номер прерывания в строку
  char buf[16];
  int i = 0;
  if (interrupt_no == 0) {
    buf[i++] = '0';
  } else {
    unsigned int n = interrupt_no;
    while (n > 0) {
      buf[i++] = '0' + (n % 10);
      n /= 10;
    }
  }
  buf[i] = '\0';
  for (int j = 0, k = i - 1; j < k; j++, k--) {
    char temp = buf[j];
    buf[j] = buf[k];
    buf[k] = temp;
  }

  // Преобразовать код ошибки в шестнадцатеричный
  char err_buf[16];
  i = 0;
  unsigned int e = error_code;
  if (e == 0) {
    err_buf[i++] = '0';
  } else {
    while (e > 0) {
      int digit = e % 16;
      err_buf[i++] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
      e /= 16;
    }
  }
  err_buf[i] = '\0';
  for (int j = 0, k = i - 1; j < k; j++, k--) {
    char temp = err_buf[j];
    err_buf[j] = err_buf[k];
    err_buf[k] = temp;
  }

  // // Print error
  ConsoleForeground(CONSOLE_COLOR_RED);
  printf("Exception: %s\n", buf);
  printf("Error: 0x%i %s", 11 + strlen(buf), err_buf);
  printf("The system is on hold %i", 22 + strlen(buf) + strlen(err_buf));
  // print_string("Exception: ", 0, 0, 0x0C);
  // print_string(buf, 11, 0, 0x0C);
  // print_string(" (Error: 0x", 11 + strlen(buf), 0, 0x0C);
  // print_string(err_buf, 22 + strlen(buf), 0, 0x0C);
  // print_string(") - Halting", 22 + strlen(buf) + strlen(err_buf), 0, 0x0C);

  // Halt
  // while (1) {
  //   __asm__ volatile("hlt");
  // }
}
