#pragma once
#include <System/Process/Process.h>
#include <stddef.h>
#include <types.h>

extern Task *CurrentTask;
extern Task *HeadTask;

// Структура кадра, запечатанного на стеке задачи
typedef struct {
  // 1. Регистры общего назначения (сохраняемые нами вручную через push)
  uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
  uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;

  // 2. Аппаратный кадр (заталкивается процессором автоматически при прерывании)
  uint64_t rip; // Точка входа в функцию задачи
  uint64_t cs;  // Селектор кода ядра (из Limine GDT, обычно 0x08)
  uint64_t
      rflags;   // Флаги процессора (обязательно 0x202 для включения прерываний)
  uint64_t rsp; // Указатель на стек задачи
  uint64_t ss;  // Селектор данных ядра (обычно 0x10)
} __attribute__((packed)) trap_frame_t;

void AddTask(Task *NewTask);
void RemoveTask(Task *task);
Task FindeTask(u32 id);

u64 Schedule(u64 current_rsp);
