#!/bin/bash

set -e

CC=mips64r5900el-ps2-elf-gcc
OBJCOPY=mips64r5900el-ps2-elf-objcopy
NM=mips64r5900el-ps2-elf-nm

SOURCE=main.c
OBJECT=bin/SOCOM.o
ELF=bin/SOCOM.elf
BINARY=bin/SOCOM.bin
LINKER=linker/cave.ld
PNACH=bin/SOCOM.pnach

# Location where the entire linked payload is written.
CODE_CAVE=0x00097000

# Original game JAL locations.
CHECKDISHOOT_HOOK_ADDR=0x001EBF20
HANDLEFIREWEAPON_HOOK_ADDR=0x002B927C

# Fill in RETAIL call sites when reversed (not native function entry addresses).
# Zero leaves that additional hook uninstalled; the original two hooks remain.
# calls to zVid_ZTestOn , recoTick & CHUD_PauseGame
MENU_DRAW_HOOK_ADDR=0x0017C5FC
MENU_INPUT_HOOK_ADDR=0x0017C358
MENU_PAUSE_HOOK_ADDR=0x0017C37C
MENU_PAUSE_HOOK_ADDR_2=0x0017C590
MENU_PAUSE_HOOK_ADDR_3=0x001EA45C

if [ -z "$PS2SDK" ]; then
    echo "Error: PS2SDK is not set. Run the PS2 toolchain environment script first."
    exit 1
fi

mkdir -p bin
# Native C interop settings. main.c includes ui.c, so still one object file.
CFLAGS='-O2 -std=gnu11 -G0 -mno-abicalls -fno-pic -ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fomit-frame-pointer
-ffixed-$16 -ffixed-$17 -ffixed-$18 -ffixed-$19 -ffixed-$20 -ffixed-$21 -ffixed-$22 -ffixed-$23 -ffixed-$28 -ffixed-$30
-ffixed-$f20 -ffixed-$f21 -ffixed-$f22 -ffixed-$f23 -ffixed-$f24 -ffixed-$f25 -ffixed-$f26 -ffixed-$f27 -ffixed-$f28 -ffixed-$f29 -ffixed-$f30 -ffixed-$f31'

echo "[1/4] Compiling..."
$CC $CFLAGS -c "$SOURCE" -o "$OBJECT"

echo "[2/4] Linking..."
$CC -G0 -mno-abicalls -fno-pic \
    -nostdlib \
    -nostartfiles \
    -T "$LINKER" \
    "$OBJECT" \
    -o "$ELF"

#
# Resolve the actual linked addresses of our hook functions.
#
CHECKDISHOOT_ADDR=$(
    $NM -n "$ELF" |
    awk '$3 == "hk_CheckDIShoot" { print "0x"$1; exit }'
)

HANDLEFIREWEAPON_ADDR=$(
    $NM -n "$ELF" |
    awk '$3 == "hk_HandleFireWeapon" { print "0x"$1; exit }'
)

FEATURESET_ADDR=$(
    $NM -n "$ELF" |
    awk '$3 == "m_featureset" { print "0x"$1; exit }'
)

if [ -z "$CHECKDISHOOT_ADDR" ]; then
    echo "Error: Could not find hk_CheckDIShoot in $ELF"
    exit 1
fi

if [ -z "$HANDLEFIREWEAPON_ADDR" ]; then
    echo "Error: Could not find hk_HandleFireWeapon in $ELF"
    exit 1
fi

if [ -z "$FEATURESET_ADDR" ]; then
    echo "Error: Could not find m_featureset in $ELF"
    exit 1
fi

echo
echo "Resolved hook addresses:"
echo "  hk_CheckDIShoot   = $CHECKDISHOOT_ADDR"
echo "  hk_HandleFireWeapon = $HANDLEFIREWEAPON_ADDR"
echo "  m_featureset      = $FEATURESET_ADDR"
echo




# Additional menu symbols use the same nm lookup as your original hooks.
MUTABLE_START=$($NM -n "$ELF" | awk '$3 == "__mutable_start" { print "0x"$1; exit }')
MUTABLE_END=$($NM -n "$ELF" | awk '$3 == "__mutable_end" { print "0x"$1; exit }')
MENU_DRAW_ADDR=$($NM -n "$ELF" | awk '$3 == "MenuHook97205" { print "0x"$1; exit }')
MENU_INPUT_ADDR=$($NM -n "$ELF" | awk '$3 == "MenuInputHook97205" { print "0x"$1; exit }')
MENU_PAUSE_ADDR=$($NM -n "$ELF" | awk '$3 == "MenuPauseHook97205" { print "0x"$1; exit }')
for address in "$MUTABLE_START" "$MUTABLE_END" "$MENU_DRAW_ADDR" "$MENU_INPUT_ADDR" "$MENU_PAUSE_ADDR"; do
    if [ -z "$address" ]; then echo "Error: missing linked menu/state symbol"; exit 1; fi
done
MENU_ARGS=(--mutable-range "$MUTABLE_START" "$MUTABLE_END")
if (( MENU_DRAW_HOOK_ADDR )); then MENU_ARGS+=(--hook "$MENU_DRAW_HOOK_ADDR" "$MENU_DRAW_ADDR" --restore "$MENU_DRAW_HOOK_ADDR" 0x0C0C7B64); fi
if (( MENU_INPUT_HOOK_ADDR )); then MENU_ARGS+=(--hook "$MENU_INPUT_HOOK_ADDR" "$MENU_INPUT_ADDR" --restore "$MENU_INPUT_HOOK_ADDR" 0x0C0D4ADC); fi
for pause_site in "$MENU_PAUSE_HOOK_ADDR" "$MENU_PAUSE_HOOK_ADDR_2" "$MENU_PAUSE_HOOK_ADDR_3"; do
    if (( pause_site )); then MENU_ARGS+=(--hook "$pause_site" "$MENU_PAUSE_ADDR" --restore "$pause_site" 0x0C0F677C); fi
done
if (( ! MENU_DRAW_HOOK_ADDR || ! MENU_INPUT_HOOK_ADDR || ! MENU_PAUSE_HOOK_ADDR || ! MENU_PAUSE_HOOK_ADDR_2 || ! MENU_PAUSE_HOOK_ADDR_3 )); then
    echo "Menu call-site offsets are pending in build.sh; zero entries are not patched."
fi

echo "[3/4] Extracting payload..."
$OBJCOPY \
    -O binary \
    -j .hook \
    -j .hook_teleport \
    -j .hook_menu \
    -j .hook_input \
    -j .hook_pause \
    -j .text \
    -j .rodata \
    -j .cheats \
    -j .data \
    -j .bss \
    -j .menu_state \
    "$ELF" \
    "$BINARY"

echo "[4/4] Generating PNACH..."
python3 GENPnach.py \
    "$CODE_CAVE" \
    "$BINARY" \
    "$PNACH" \
    "$FEATURESET_ADDR" \
    "$CHECKDISHOOT_HOOK_ADDR" \
    "$CHECKDISHOOT_ADDR" \
    "$HANDLEFIREWEAPON_HOOK_ADDR" \
    "$HANDLEFIREWEAPON_ADDR" \
    "${MENU_ARGS[@]}"

echo
echo "Build complete."
echo "Output: $PNACH"