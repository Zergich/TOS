#include <System/MemoryManager/PMM.h>
#include <System/MemoryManager/VMM.h>
#include <System/MemoryManager/kmalloc/kmalloc.h>
#include <System/Sheduler/sheduler.h>
#include <arch/x86_64/interrupts.h>
#include <arch/x86_64/io.h>
#include <libs/time.h>
#include <types.h>

Task *CurrentTask = NULL;
Task *HeadTask = NULL;
Task *PID0_Prt = NULL;

static u64 next_stack_vaddr = 0xFFFFFF0000000000;
volatile u16 PitTicks = 0;
static bool IsFirstTask = true;

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

void IdleTask() {
  while (true) {
    asm volatile("hlt");
  }
}

void TaskExit() {
  asm volatile("cli");
  CurrentTask->state = TASK_DEAD;
  asm volatile("int $0x20");
  while (true) {
    asm volatile("hlt");
  }
}

Task *CreateHideTask(void (*entry_point)()) {
  Task *t = kmalloc(sizeof(Task));

  u64 max_stack_size = 2 * 1024 * 1024;
  t->stack_base = next_stack_vaddr;
  t->stack_limit = t->stack_base - max_stack_size;
  next_stack_vaddr -= max_stack_size;

  u64 first_page_vaddr = t->stack_base - PAGE_SIZE;
  uintptr_t phys = pmm_alloc_frame();
  vmm_map_page(&kernel_space, first_page_vaddr, phys,
               PTE_PRESENT | PTE_WRITABLE);

  u64 stack_top = t->stack_base;
  stack_top &= ~0xF;
  stack_top -= 8;

  u64 *return_ptr = (u64 *)stack_top;
  *return_ptr = (u64)TaskExit;

  trap_frame_t *frame = (trap_frame_t *)(stack_top - sizeof(trap_frame_t));

  u16 current_cs, current_ss;
  __asm__ volatile("mov %%cs, %0" : "=r"(current_cs));
  __asm__ volatile("mov %%ss, %0" : "=r"(current_ss));

  frame->rip = (u64)entry_point;
  frame->cs =
      (u64)current_cs; // Важно: каст к u64 для правильного размера на стеке
  frame->ss = (u64)current_ss;
  frame->rsp = stack_top;
  frame->rflags = 0x202;

  // Обнуляем регистры
  frame->rax = frame->rbx = frame->rcx = frame->rdx = 0;
  frame->rsi = frame->rdi = frame->rbp = 0;
  frame->r8 = frame->r9 = frame->r10 = frame->r11 = 0;
  frame->r12 = frame->r13 = frame->r14 = frame->r15 = 0;

  t->rsp = (u64)frame;
  t->state = TASK_READY; // Инициализируем статус!

  return t;
}

// Избавились от дублирования кода!
Task *CreateTask(void (*entry_point)()) {
  Task *t = CreateHideTask(entry_point);
  AddTask(t);
  return t;
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
