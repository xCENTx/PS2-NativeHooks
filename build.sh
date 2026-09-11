#!/bin/bash

set -e

CC=mips64r5900el-ps2-elf-gcc
OBJCOPY=mips64r5900el-ps2-elf-objcopy

SOURCE=main.c
OBJECT=bin/SOCOM.o
ELF=bin/SOCOM.elf
BINARY=bin/SOCOM.bin
LINKER=linker/cave.ld
PNACH=bin/SOCOM.pnach

HOOK_ADDR=0x001EBF20
CODE_CAVE=0x00097000

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

echo "[3/4] Extracting payload..."
$OBJCOPY -O binary "$ELF" "$BINARY"

echo "[4/4] Generating PNACH..."
python3 GENPnach.py \
    "$HOOK_ADDR" \
    "$CODE_CAVE" \
    "$BINARY" \
    "$PNACH"

echo