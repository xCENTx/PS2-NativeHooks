SECTIONS
{
    . = 0x00097000;

    .text :
    {
        *(.text*)
    }

    .rodata :
    {
        *(.rodata*)
        *(.rodata.*)
    }

    .data :
    {
        *(.data*)
        *(.data.*)
    }

    .bss :
    {
        *(.bss*)
    }
}