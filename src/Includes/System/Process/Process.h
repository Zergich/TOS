
#pragma once
#include <System/Process/FdTable.h>
#include <stddef.h>
#include <types.h>
typedef enum { TASK_READY, TASK_RUNNING, TASK_BLOCKED, TASK_DEAD } TaskState;

typedef struct taskinfo {
  u32 id;
  u32 *name;
  u64 rsp;            // указатель на стек (там где регистры)
  void *stack_bottom; // Указатель на выделенную память стека
                      // (для free())
  u64 stack_base;     // Самый верх стека (откуда он начинает расти вниз)
  u64 stack_limit;    // Максимальная нижняя граница (например, 2 МБ)
  TaskState state;
  // Ресурсы процесса
  FdTable Files;

  struct taskinfo *next;
  struct taskinfo *back;

} Task;

Task *CreateTask(void (*entry_point)());

Task *CreateHideTask(void (*entry_point)());
void IdleTask();
extern Task *PID0_Prt;
