OUTPUT_ARCH(mips)
ENTRY(MenuHook97205)
SECTIONS {
 . = 0x97000;
 .hook_menu : { KEEP(*(.hook_menu)) KEEP(*(.hook)) }
 . = ALIGN(16);
 .hook_input : { KEEP(*(.hook_input)) }
 . = ALIGN(16);
 .hook_pause : { KEEP(*(.hook_pause)) }
 . = ALIGN(16);
 .hook_teleport : { KEEP(*(.hook_teleport)) }
 .text : { *(.text*) }
 .rodata : { *(.rodata*) *(.srodata*) }
 . = ALIGN(16);
 __data_load = .;
 .data 0x9F000 : AT(__data_load) {
   __data_start = .; *(.data*) *(.sdata*) __data_end = .;
 }
 .bss (NOLOAD) : {
   __bss_start = .; *(.bss*) *(.sbss*) *(.cheats) *(COMMON) __bss_end = .;
 }
 .menu_state (NOLOAD) : { *(.menu_state) }
 .got : { *(.got*) }
 /DISCARD/ : { *(.comment) *(.note*) *(.pdr) *(.reginfo) *(.MIPS.abiflags) *(.eh_frame*) }
 ASSERT(__data_load + SIZEOF(.data) <= 0x9F000, "Payload overlaps runtime state")
 ASSERT(. <= 0xA0000, "Runtime state overflow")
 ASSERT(SIZEOF(.got)==0, "Unexpected GP/GOT data")
}
