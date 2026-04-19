global long_mode_start
extern kernel_main

extern mb2_magic   
extern mb2_info_ptr 

section .text
bits 64
long_mode_start:
    ; load null into all data segment registers
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; параметры для kmain
    mov edi, dword [mb2_magic]      ; 1-й аргумент для main (0x36D76289)
    mov esi, dword [mb2_info_ptr]   ; 2-й аргумент для main (указатель на структуру)

	call kernel_main
    hlt
