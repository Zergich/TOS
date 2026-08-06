#include <System/MemoryManager/kmalloc/kmalloc.h>
#include <System/Sheduler/sheduler.h>
#include <arch/x86_64/interrupts.h>
#include <arch/x86_64/io.h>
#include <libs/time.h>
#include <types.h>

Task *CurrentTask = NULL;
Task *HeadTask = NULL;

void AddTask(Task *NewTask) {
  if (HeadTask == NULL) {
    HeadTask = NewTask;
    CurrentTask = NewTask;

    // кольцо из одного элемента укахывает само на себя
    NewTask->next = HeadTask;
    NewTask->back = HeadTask;
    return;
  }
  Task *LastTask =
      HeadTask->back; // Последний элемент это просто предыдущий у головы

  LastTask->next = NewTask;
  NewTask->back = LastTask;

  NewTask->next = HeadTask;
  HeadTask->back = NewTask;
}

void RemoveTask(Task *task) {
  if (task->next == task) {
    HeadTask = NULL;
    CurrentTask = NULL;
  }
  // Замыкаем соседей между собой, вычеркивая текущую задачу
  task->back->next = task->next;
  task->next->back = task->back;

  // Если удаляемая задача была головой списка — сдвигаем голову
  if (HeadTask == task) {
    HeadTask = task->next;
  }

  // Если удаляем текущую исполняемую задачу — сдвигаем current_task
  if (CurrentTask == task) {
    CurrentTask = task->next;
  }

  // Зачищаем указатели удаленного элемента
  task->next = NULL;
  task->back = NULL;
}

static u32 next_pid = 1;

Task *CreateTask(void (*entry_point)()) {
  // 1. Выделяем память под структуру задачи
  Task *t = (Task *)kmalloc(sizeof(Task));
  t->id = next_pid++;
  t->state = TASK_READY;

  // 2. Выделяем 16 КБ под стек задачи
  u64 stack_size = 4096 * 4;
  void *stack_mem = kmalloc(stack_size);
  t->stack_bottom = stack_mem;

  // 3. Вычисляем ВЕРШИНУ стека (так как стек в x86_64 растёт СВЕРХУ ВНИЗ)
  u64 stack_top = (u64)stack_mem + stack_size;

  // Выравниваем верхушку стека по 16 байт (требование System V ABI)
  stack_top &= ~0xF;

  // 4. Размещаем фальшивый кадр на самой вершине стека
  trap_frame_t *frame = (trap_frame_t *)(stack_top - sizeof(trap_frame_t));

  // 5. Заполняем аппаратную часть кадра
  frame->rip = (u64)entry_point; // Куда прыгнет процессор при iretq
  // Читаем текущие рабочие CS и SS, на которых сейчас работает ядро
  u16 current_cs, current_ss;
  __asm__ volatile("mov %%cs, %0" : "=r"(current_cs));
  __asm__ volatile("mov %%ss, %0" : "=r"(current_ss));

  // 5. Заполняем аппаратную часть кадра
  frame->rip = (u64)entry_point;
  frame->cs = current_cs; // Берем реальный CS!
  frame->ss = current_ss; // Берем реальный SS!
  frame->rsp = stack_top;
  frame->rflags = 0x202;

  // 5. Заполняем аппаратную часть кадра
  frame->rip = (u64)entry_point;
  frame->cs = current_cs; // Берем реальный CS!
  frame->ss = current_ss; // Берем реальный SS!
  frame->rsp = stack_top;
  frame->rflags = 0x202; // 6. Обнуляем регистры общего назначения
  frame->rax = 0;
  frame->rbx = 0;
  frame->rcx = 0;
  frame->rdx = 0;
  frame->rsi = 0;
  frame->rdi = 0;
  frame->rbp = 0;
  frame->r8 = 0;
  frame->r9 = 0;
  frame->r10 = 0;
  frame->r11 = 0;
  frame->r12 = 0;
  frame->r13 = 0;
  frame->r14 = 0;
  frame->r15 = 0;

  // 7. Сохраняем текущую верхушку стека (с кадра) в структуру задачи
  t->rsp = (u64)frame;

  // 8. Вставляем задачу в твой кольцевой список
  AddTask(t);

  return t;
}
volatile u16 PitTicks = 0; // тики и так сбрасываются переполнением
static bool IsFirstTask = true;
u64 Schedule(u64 current_rsp) {
  //  старый код для работы таймера
  PitTicks++;
  if (PitTicks % 1000 == 0)
    Timepit.PitTimerSecondsUp++;
  Timepit.PitTimerMiliSecondsUp++;
  outb(PIC1_COMMAND, PIC_EOI);

  //------------------------------

  if (CurrentTask == NULL) {
    return current_rsp;
  }

  // --- ФИКС: При первом запуске НЕ сохраняем старый стек! ---
  if (IsFirstTask) {
    IsFirstTask = false;
    return CurrentTask->rsp; // Сразу отдаём стек новой задачи!
  }

  // 1. Сохраняем RSP текущей задачи
  CurrentTask->rsp = current_rsp;

  // 2. Переходим к следующей задаче по кольцу
  CurrentTask = CurrentTask->next;

  // 3. Возвращаем RSP новой задачи
  return CurrentTask->rsp;
}
