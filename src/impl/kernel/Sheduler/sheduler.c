#include <System/MemoryManager/PMM.h>
#include <System/MemoryManager/VMM.h>
#include <System/MemoryManager/kmalloc/kmalloc.h>
#include <System/Process/Process.h>
#include <System/Sheduler/sheduler.h>
#include <arch/x86_64/interrupts.h>
#include <arch/x86_64/io.h>
#include <libs/time.h>
#include <types.h>

volatile u16 PitTicks = 0;

static bool IsFirstTask = true;
Task *CurrentTask = NULL;
Task *HeadTask = NULL;
Task *PID0_Prt = NULL;

void AddTask(Task *NewTask) {
  if (HeadTask == NULL) {
    HeadTask = NewTask;
    CurrentTask = NewTask;
    NewTask->next = HeadTask;
    NewTask->back = HeadTask;
    return;
  }
  Task *LastTask = HeadTask->back;

  LastTask->next = NewTask;
  NewTask->back = LastTask;

  NewTask->next = HeadTask;
  HeadTask->back = NewTask;
}

void RemoveTask(Task *task) {
  if (task == NULL)
    return;

  // 1. Если это был единственный элемент в кольце
  if (task->next == task) {
    HeadTask = NULL;
    CurrentTask = NULL;
    task->next = NULL;
    task->back = NULL;
    return;
  }

  // 2. Вычеркиваем задачу из кольца
  task->back->next = task->next;
  task->next->back = task->back;

  if (HeadTask == task) {
    HeadTask = task->next;
  }

  if (CurrentTask == task) {
    CurrentTask = task->next;
  }

  task->next = NULL;
  task->back = NULL;
}

void DeadTaskReaper(Task *DeadTask);
static Task *TaskToFree = NULL;
u64 Schedule(u64 current_rsp) {
  PitTicks++;
  if (PitTicks % 1000 == 0)
    Timepit.PitTimerSecondsUp++;
  Timepit.PitTimerMiliSecondsUp++;
  outb(PIC1_COMMAND, PIC_EOI);

  // 1. Первый запуск системы
  if (IsFirstTask) {
    IsFirstTask = false;
    if (HeadTask != NULL) {
      return CurrentTask->rsp;
    }
    return PID0_Prt->rsp;
  }
  if (TaskToFree != NULL) {
    DeadTaskReaper(TaskToFree);
    TaskToFree = NULL;
  }

  // 2. Если очередь пуста — мы работаем в контексте PID0
  if (HeadTask == NULL || CurrentTask == NULL) {
    PID0_Prt->rsp = current_rsp; // ОБЯЗАТЕЛЬНО сохраняем текущий стек PID0!
    return PID0_Prt->rsp;
  }

  // 3. Если текущая задача завершилась (TASK_DEAD)
  if (CurrentTask->state == TASK_DEAD) {
    Task *dead_task = CurrentTask;

    // Перемещаемся на следующую перед удалением
    CurrentTask = CurrentTask->next;
    RemoveTask(dead_task);
    TaskToFree = dead_task;
    // Если задач не осталось — переключаемся на PID0
    if (HeadTask == NULL || CurrentTask == NULL) {
      return PID0_Prt->rsp;
    }

    // Возвращаем стек новой задачи (стек умершей НЕ сохраняем)
    return CurrentTask->rsp;
  }

  // 4. Обычная ротация живых задач
  CurrentTask->rsp = current_rsp;
  CurrentTask = CurrentTask->next;

  return CurrentTask->rsp;
}

void DeadTaskReaper(Task *DeadTask) {
  if (DeadTask == NULL)
    return;
  u64 FirstPage = DeadTask->stack_base -
                  PAGE_SIZE; // для поиска виртуального адреса первой страницы
  u64 PhysAddr = vmm_virt_to_phys(&kernel_space, FirstPage);
  vmm_unmap_page(&kernel_space, FirstPage);
  pmm_free_frame(PhysAddr);
  kfree(DeadTask);
}
