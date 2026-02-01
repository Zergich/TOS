bits 64

global idt_load

section .text

idt_load:
    ; Первый аргумент (указатель на idt_ptr) в регистре RDI
    mov rdi, [rsp + 8]   ; в 64-битных вызовах, параметр передается через RDI
    
    ; загрузка IDT
    lidt [rdi]
    ret
