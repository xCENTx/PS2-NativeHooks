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
$OBJCOPY \
    -O binary \
    -j .text \
    -j .rodata \
    "$ELF" \
    "$BINARY"

echo "[4/4] Generating PNACH..."
python3 ELFPatch.py \
    "$HOOK_ADDR" \
    "$CODE_CAVE" \
    "$BINARY" \
    "$PNACH"

echo
# echo "Done!"
# echo "Generated: $PNACH"