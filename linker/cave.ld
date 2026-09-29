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

    . = ALIGN(4);
    __mutable_start = .;

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
    .menu_state :
    {
        *(.menu_state)
        BYTE(0)
        . = ALIGN(4);
    }
    __mutable_end = .;
    /DISCARD/ : { *(.comment) *(.note*) *(.pdr) *(.reginfo) *(.MIPS.abiflags) *(.eh_frame*) }
}
