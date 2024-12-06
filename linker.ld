ENTRY(loader_entry)

SECTIONS {
    . = 0x1000;   /* O bootloader começa em 0x1000 */

    .text : ALIGN(4K) {
        *(.text*)
    }

    .rodata : ALIGN(4K) {
        *(.rodata*)
    }

    .data : ALIGN(4K) {
        *(.data*)
    }

    .bss : ALIGN(4K) {
        *(COMMON)
        *(.bss*)
    }

    /* Defina a memória onde o kernel será carregado */
    .kernel : {
        *(.kernel*)
    }
}
