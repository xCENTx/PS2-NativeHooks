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

if [ -z "$PS2SDK" ]; then
    echo "Error: PS2SDK is not set. Run the PS2 toolchain environment script first."
    exit 1
fi

echo "[1/4] Compiling..."
$CC -O2 -c "$SOURCE" -o "$OBJECT"

echo "[2/4] Linking..."
$CC \
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
    echo "Error: Could not find hk_WillFireWeapon in $ELF"
    exit 1
fi

if [ -z "$FEATURESET_ADDR" ]; then
    echo "Error: Could not find m_featureset in $ELF"
    exit 1
fi

echo
echo "Resolved hook addresses:"
echo "  hk_CheckDIShoot   = $CHECKDISHOOT_ADDR"
echo "  hk_WillFireWeapon = $HANDLEFIREWEAPON_ADDR"
echo "  m_featureset      = $FEATURESET_ADDR"
echo



echo "[3/4] Extracting payload..."
$OBJCOPY \
    -O binary \
    -j .hook \
    -j .hook_teleport \
    -j .text \
    -j .rodata \
    -j .cheats \
    -j .data \
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
    "$HANDLEFIREWEAPON_ADDR"

echo
echo "Build complete."
echo "Output: $PNACH"