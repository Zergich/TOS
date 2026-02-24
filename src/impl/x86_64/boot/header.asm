section .multiboot_header
header_start:
; здесь определяется магияеское число которое ищет загрузчик и по нему определяет что перед ним ось
  dd 0xe85250d6 ; multiboot2
  dd 0; переход в защищенный режим i386
  dd header_end - header_start: ; длинна заголовка
  ;контрольная сумма
  dd 0x100000000 - (0xe85250d6 +0+(header_end - header_start))

  ;конечный тег
  dw 0
  dw 0
  dd 8
header_end:


; для граба и полноразмерного экрана
; section .multiboot_header
; align 8
; header_start:
;     ; Магическое число Multiboot 2
;     dd 0xE85250D6
;     ; Архитектура (0 - i386)
;     dd 0
;     ; Длина заголовка
;     dd header_end - header_start
;     ; Контрольная сумма
;     dd -(0xE85250D6 + 0 + (header_end - header_start))
;
;     ; --- ТЕГ ЗАПРОСА ФРЕЙМБУФЕРА ---
;     ; Структура: type (u16), flags (u16), size (u32)
;    
;     dw 5              ; Тип: 5 (Framebuffer request) -> ИСПРАВЛЕНО: dw (2 байта)
;     dw 1              ; Флаги: 1 (required)           -> ИСПРАВЛЕНО: dw (2 байта)
;     dd 20             ; Размер: 20 байт (всего)
;
;     ; Параметры режима
;     dd 0              ; width  (0 = любое)
;     dd 0              ; height (0 = любое)
;     dd 32             ; depth  (32 бита на пиксель)
;
;     ; --- КОНЕЦ ЗАГОЛОВКА ---
;     ; Структура: type (u16), flags (u16), size (u32)
; align 8
;     dw 0              ; Тип 0 (Конец)
;     dw 0              ; Флаги (зарезервировано, 0)
;     dd 8              ; Размер (8 байт)
; header_end:
