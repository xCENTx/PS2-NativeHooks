SECTIONS
{
    . = 0x00097000;

    .hook :
    {
        *(.hook)
    }

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