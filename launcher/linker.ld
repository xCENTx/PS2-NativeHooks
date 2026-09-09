/*
 * Keep the launcher below normal game memory so SifLoadElf can populate
 * SOCOM's segments at 0x00100000+ without overwriting the running launcher.
 * The payload code cave at 0x00097000 remains below this region.
 */
ENTRY(__start)

MEMORY
{
    launcher (rwx) : ORIGIN = 0x01EE8000, LENGTH = 0x00118000
}

END_LAUNCHER_REGION = ORIGIN(launcher) + LENGTH(launcher);

PHDRS
{
    text PT_LOAD;
}

SECTIONS
{
    .text :
    {
        _ftext = .;
        *(.text)
        *(.text.*)
        *(.gnu.linkonce.t*)
        KEEP(*(.init))
        KEEP(*(.fini))
        QUAD(0)
    } >launcher :text

    PROVIDE(_etext = .);
    PROVIDE(etext = .);

    .ctors ALIGN(16) :
    {
        KEEP(*crtbegin*.o(.ctors))
        KEEP(*(EXCLUDE_FILE(*crtend*.o) .ctors))
        KEEP(*(SORT(.ctors.*)))
        KEEP(*(.ctors))
    } >launcher

    .dtors ALIGN(16) :
    {
        KEEP(*crtbegin*.o(.dtors))
        KEEP(*(EXCLUDE_FILE(*crtend*.o) .dtors))
        KEEP(*(SORT(.dtors.*)))
        KEEP(*(.dtors))
    } >launcher

    .reginfo : { *(.reginfo) } >launcher

    .data ALIGN(128) :
    {
        _fdata = .;
        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d*)
        SORT(CONSTRUCTORS)
    } >launcher

    .rodata ALIGN(128) :
    {
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r*)
    } >launcher

    .rdata ALIGN(128) : { *(.rdata) } >launcher
    .gcc_except_table ALIGN(128) : { *(.gcc_except_table) } >launcher

    _gp = ALIGN(128) + 0x7ff0;
    .lit4 ALIGN(128) : { *(.lit4) } >launcher
    .lit8 ALIGN(128) : { *(.lit8) } >launcher

    .sdata ALIGN(128) :
    {
        *(.sdata)
        *(.sdata.*)
        *(.gnu.linkonce.s*)
    } >launcher

    _edata = .;
    PROVIDE(edata = .);

    .sbss ALIGN(128) :
    {
        _fbss = .;
        *(.sbss)
        *(.sbss.*)
        *(.gnu.linkonce.sb*)
        *(.scommon)
    } >launcher

    .bss ALIGN(128) :
    {
        *(.bss)
        *(.bss.*)
        *(.gnu.linkonce.b*)
        *(COMMON)
    } >launcher

    _end_bss = .;
    _end = .;
    PROVIDE(end = .);

    .spad 0x70000000 : { *(.spad) }

    PROVIDE(_stack_size = 32 * 1024);
    PROVIDE(_stack = END_LAUNCHER_REGION - _stack_size);
    PROVIDE(_heap_size = -1);
}
