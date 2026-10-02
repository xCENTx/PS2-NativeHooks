SECTIONS
{
    . = 0x00097000;

    .bootstrap :
    {
        *(.bootstrap)
    }

    .hook_menu : { *(.hook_menu) }
    .hook_input : { *(.hook_input) }
    .hook_pause : { *(.hook_pause) }

    .text :
    {
        *(.text*)
    }

    .rodata :
    {
        *(.rodata*)
        *(.rodata.*)
        *(.srodata*)
    }

    . = ALIGN(16);
    __mutable_start = .;

    .hooks :
    {
        *(.hooks)
    }

    .cheats :
    {
        *(.cheats)
    }

    .data :
    {
        *(.data*)
        *(.data.*)
        *(.sdata*)
    }

    .bss :
    {
        *(.bss*)
        *(.sbss*)
        *(COMMON)
    }

    .thread_stack :
    {
        . = ALIGN(16);
        *(.thread_stack)
        . = ALIGN(16);
    }

    .menu_state :
    {
        *(.menu_state)
        BYTE(0)
        . = ALIGN(4);
    }

    __mutable_end = .;

    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
        *(.pdr)
        *(.reginfo)
        *(.MIPS.abiflags)
        *(.eh_frame*)
    }
}