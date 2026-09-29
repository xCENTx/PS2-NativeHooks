SECTIONS
{
    . = 0x00097000;

    .hook :
    {
        *(.hook)
    }

    .hook_teleport :
    {
        *(.hook_teleport)
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

    .cheats :
    {
        *(.cheats)
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