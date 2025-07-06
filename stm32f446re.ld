ENTRY(reset_handler)

MEMORY {
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    SRAM1 (rwx) : ORIGIN = 0x20000000, LENGTH = 112K
    SRAM2 (rwx) : ORIGIN = 0x2001C000, LENGTH = 16K
}

_sstack = ORIGIN(SRAM1) + LENGTH(SRAM1);

SECTIONS {
    .isr_vector : {
        KEEP(*(.isr_vector))
    } > FLASH

    .text : {
        _stext = ALIGN(4);

        *(.text)
        *(.rodata)

        _etext = ALIGN(4);
    } > FLASH

    .data : {
        _sdata = ALIGN(4);

        *(.data)

        _edata = ALIGN(4);
    } > SRAM1 AT > FLASH

    .bss : { 
        _sbss = ALIGN(4);

        *(.bss)

        _ebss = ALIGN(4);
        *(.bss)
    } > SRAM1
}

