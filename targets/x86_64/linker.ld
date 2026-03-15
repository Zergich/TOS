ENTRY(start)

/* Явно определяем типы сегментов (Program Headers) с правами */
PHDRS
{
    text PT_LOAD FLAGS(0x5);   /* 0x5 = Read + Execute (R-X) */
    data PT_LOAD FLAGS(0x6);   /* 0x6 = Read + Write (RW-) */
    bss  PT_LOAD FLAGS(0x6);   /* 0x6 = Read + Write (RW-) */
}

SECTIONS
{
    . = 1M;

    /* Секция загрузки. Привязываем к текстовому сегменту (:text) */
    .boot : {
        KEEP(*(.multiboot_header))
    } : text

    /* Секция кода. Привязываем к текстовому сегменту (:text) */
    .text : {
        *(.text)
        *(.text.*)
    } : text

    /* Секция только для чтения. Привязываем к текстовому сегменту (:text) */
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } : text

    /* Секция данных. Привязываем к сегменту данных (:data) */
    .data : {
        *(.data)
        *(.data.*)
        /* Помещаем сюда "сиротские" секции (например, .jcr), чтобы не создавались новые сегменты */
        *(.jcr)
    } : data

    /* Секция BSS. Привязываем к сегменту данных (:data) */
    .bss : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    } : data

    /* Удаляем лишние секции, которые могут создать свои сегменты */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.eh_frame_hdr)
        *(.note.gnu.build-id)
    }
}
