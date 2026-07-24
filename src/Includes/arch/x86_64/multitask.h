#pragma once
#include <types.h>

typedef struct {
  u64 rsp;
} Task;

// Объявляем переменные как extern (ПАМЯТЬ НЕ ВЫДЕЛЯЕТСЯ)
extern u8 Stack_A[4096];
extern u8 Stack_B[4096];

extern Task task_A;
extern Task task_B;
extern Task task_Main;

extern Task *current_task;

// Прототипы функций
void switch_context(Task *old, Task *new);
void init_task(Task *task, u8 *stack, u32 stack_size, void (*entry_point)());
void yield();
void StartMultitasking();
