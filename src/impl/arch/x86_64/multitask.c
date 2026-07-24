#include <ConsoleIO/print.h>
#include <arch/x86_64/multitask.h>
#include <libs/time.h>
#include <types.h>

u8 Stack_A[4096];
u8 Stack_B[4096];

Task task_A;
Task task_B;
Task task_Main;

Task *current_task = 0;
// __attribute__((naked)) говорит компилятору не добавлять стандартные прологи
// функций, мы всё делаем чистым ассемблером.
__attribute__((naked)) void switch_context(Task *old, Task *new) {
  asm volatile(
      "push %%rbp\n"
      "push %%rbx\n"
      "push %%r12\n"
      "push %%r13\n"
      "push %%r14\n"
      "push %%r15\n"

      "mov %%rsp, (%%rdi)\n" // old->rsp = текущий RSP
      "mov (%%rsi), %%rsp\n" // текущий RSP = new->rsp

      "pop %%r15\n"
      "pop %%r14\n"
      "pop %%r13\n"
      "pop %%r12\n"
      "pop %%rbx\n"
      "pop %%rbp\n"

      "ret\n" // <--- Возвращается ТУДА, куда указывал стек новой задачи!
      ::
          : "memory");
}
void init_task(Task *task, u8 *stack, u32 stack_size, void (*entry_point)()) {
  // Стек растет вниз. Берем конец массива стека.
  u64 *sp = (u64 *)(stack + stack_size);

  // Подделываем возврат из функции switch_context
  *(--sp) = (u64)entry_point; // Адрес возврата (RIP), куда сделает ret

  // Забиваем нулями регистры rbp, rbx, r12, r13, r14, r15
  *(--sp) = 0; // rbp
  *(--sp) = 0; // rbx
  *(--sp) = 0; // r12
  *(--sp) = 0; // r13
  *(--sp) = 0; // r14
  *(--sp) = 0; // r15

  // Сохраняем получившийся указатель стека в структуру задачи
  task->rsp = (u64)sp;
}

void yield() {
  Task *old_task = current_task;

  // Переключалка: если мы A, включаем B. Если B, включаем A.
  if (current_task == &task_A) {
    current_task = &task_B;
  } else {
    current_task = &task_A;
  }

  switch_context(old_task, current_task);
}

// Задачи Линуса Торвальдса:
void TaskA_Code() {
  while (1) {
    print("A");
    Timepit.Sleep(500); // Чтобы консоль не улетела в космос
    yield();            // Отдаем процессорное время
  }
}

void TaskB_Code() {
  while (1) {
    print("B");
    Timepit.Sleep(500);
    yield(); // Отдаем процессорное время
  }
}

void StartMultitasking() {
  print("Starting Torvalds Multitasking...\n");

  // 1. Создаем стеки для A и B
  init_task(&task_A, Stack_A, sizeof(Stack_A), TaskA_Code);
  init_task(&task_B, Stack_B, sizeof(Stack_B), TaskB_Code);

  // 2. Говорим, что сейчас работает Main
  current_task = &task_Main;

  // 3. Прыгаем в задачу A!
  // Состояние main() сохранится в task_Main.
  current_task = &task_A;
  switch_context(&task_Main, &task_A);

  // Этот код выполнится только если кто-то переключит контекст обратно на
  // task_Main
}
