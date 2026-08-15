#include <System/MemoryManager/PMM.h>
#include <System/MemoryManager/VMM.h>
#include <System/MemoryManager/kmalloc/kmalloc.h>
#include <System/Process/Process.h>
#include <System/Sheduler/sheduler.h>
#include <arch/x86_64/interrupts.h>
#include <arch/x86_64/io.h>
#include <libs/time.h>
#include <types.h>

static u64 next_stack_vaddr = 0xFFFFFF0000000000;

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
