bits 64
;ISR — это Interrupt Service Routine или Обработчик прерываний
; stub — это минимальный код, который вызывает вашу основную обработку

; прерывание ошибок которые не имеют код состояния по этому им присваивается код 0
%macro ISR_NOERRCODE 1
  global isr%1
    isr%1:
      cli
      push 0 ; фиктивный код ошибки (чтоб не ебали мозг обработчику)  
      push %1 ; кладем номер прерывания
      jmp exception_stub
%endmacro

; а эти уже имеют по этому прокидывается только номер прерывания

%macro ISR_ERRCODE 1
  global isr%1
    isr%1:
      cli
      push %1 ; прокидывает номер прерывания
      jmp exception_stub
%endmacro

; Определение прерываний для исключений
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


; обработка прерываний 32 и 33 (для таймера и клавиатуры)

global isr32
isr32:
  cli
  push 0
  push isr32
  jmp pit_stub ; обработка таймера

global isr33
isr33:
  cli
  push 0
  push 33 
  jmp keyboard_stub

exception_stub:
    ; Сохраняем все регистры
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

    ; Сохраняем сегменты (если нужно, но обычно они не меняются)
    ; В большинстве случаев сегменты это постоянные в 64-битных ОС, но для полноты:
    ; Здесь их можно оставить, если есть необходимость.

    ; Устанавливаем сегменты в работу (пример)
    ; mov ax, 0x10         ; базовый сегмент данных — менять по необходимости
    ; mov ds, ax
    ; mov es, ax
    ; mov fs, ax
    ; mov gs, ax

    ; Получение номера прерывания из стека
    mov rdi, [rsp + 112] ; 14 регистров * 8 байт = 112
    ; Получение error_code (если есть)
    mov rsi, [rsp + 120]

    ; Передача в обработчик
    push rsi        ; error code
    push rdi        ; номер прерывания
    extern exception_handler
    call exception_handler
    add rsp, 16     ; чистим параметры

    ; Восстановление регистров
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

    ; Восстановим сегменты, если меняли
    ; например, так:
    ; mov ax, 0x10
    ; mov ds, ax
    ; mov es, ax
    ; mov fs, ax
    ; mov gs, ax

    add rsp, 16   ; очистка переданных параметров: номер + error code
    sti
    iretq
global pit_stub
pit_stub:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Установка сегментов (если необходимо)
    ; Обычно в 64-битном режиме сегменты не меняются,
    ; особенно если всё настроено правильно.
    ; Однако, если нужно явно, то через mov rax, сегмент
    ; и вызовы системных функций.

    ; Для установки сегментов, например:
    ; mov ax, 0x10
    ; и дальше команда для установки сегментов
    ; Но в большинстве случаев этого не требуется, если настроено через GDT.

    ; Предполагаем, что у вас есть внешние обработчики:
    extern interrupt_handler_timer
    call interrupt_handler_timer

    ; Восстановление регистров
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Возврат из прерывания
    iretq
    
; Аналогично для клавиатуры
global keyboard_stub
keyboard_stub:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Аналогичная установка сегментов, если нужно
    ; Например:
    ; mov ax, 0x10
    ; mov ds, ax
    ; mov es, ax
    ; mov fs, ax
    ; mov gs, ax
    ; В 64-битных ОС обычно сегменты не меняют.

    extern interrupt_handler_keyboard
    call interrupt_handler_keyboard

    ; Восстановление
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Возвращение из прерывания
    iretq
