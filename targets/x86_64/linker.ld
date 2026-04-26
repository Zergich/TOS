OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kernel_main)

PHDRS {
    text    PT_LOAD;
    rodata  PT_LOAD;
    data    PT_LOAD;
    dynamic PT_DYNAMIC;
}

SECTIONS {
    /* Базовый адрес для Higher Half ядра */
    . = 0xffffffff80000000;

    .text : {
        *(.text .text.*)
    } :text

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    .data : {
        *(.data .data.*)
    } :data

    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /DISCARD/ : {
        *(.eh_frame)
        *(.note .note.*)
    }
}

