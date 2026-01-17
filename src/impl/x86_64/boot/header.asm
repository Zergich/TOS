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
