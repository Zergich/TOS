
bits 64

; ISR с dummy error code (нет кода ошибки)
%macro ISR_NOERRCODE 1
    global isr%1
isr%1:
    cli
    push 0                  ; фиктивный код ошибки
    push %1                 ; номер прерывания
    jmp exception_stub
%endmacro

; ISR с кодом ошибки (он проскальзывает на стек)
%macro ISR_ERRCODE 1
    global isr%1
isr%1:
    cli
    push %1                 ; номер прерывания
    jmp exception_stub
%endmacro

; Определение исключений 0-31 (соответственно с ошибкой или без)
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

; Таймер (прерывание 32)
global isr32
isr32:
    cli
    push 0
    push 32
    jmp pit_stub

; Клавиатура (прерывание 33)
global isr33
isr33:
    cli
    push 0
    push 33
    jmp keyboard_stub

exception_stub:
    ; Сохраняем регистры согласно x86-64 ABI (RBP, RBX, R12-R15 + общие)
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Сохраняем сегментные регистры, если нужно (fs/gs часто используются для TLS)
    push qword [fs:0]
    push qword [gs:0]

    ; Здесь можно установить сегменты, если надо (зависит от системы)
    ; В x86-64 обычно сегменты едины, но, если требуется, настройте fs/gs

    mov rdi, [rsp + 8*20]       ; номер прерывания (на стеке: после 15 регистров + 2 сегмента + код ошибки и номер прерывания)
    mov rsi, [rsp + 8*21]       ; код ошибки (фиктивный или настоящий)

    ; Вызов внешнего обработчика:
    extern exception_handler
    call exception_handler

    ; Восстанавливаем сегменты
    pop r15     ; gs:0
    pop r15     ; fs:0 (два раза push были? исправим ниже!)

    ; Восстанавливаем регистры
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Убираем со стека прерванные данные (номер прерывания и код ошибки)
    add rsp, 16

    sti
    iretq

pit_stub:
    ; Сохраняем регистры
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    extern interrupt_handler_timer
    call interrupt_handler_timer

    ; Восстанавливаем регистры
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16     ; убрать dummy error и номер прерывания
    sti
    iretq

keyboard_stub:
    ; Аналогично pit_stub
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    extern interrupt_handler_keyboard
    call interrupt_handler_keyboard

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16
    sti
    iretq
