#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
# Menu native offsets target debug SCUS_972.05 only.
GAME_ELF=${1:-} # Optional original ELF for hook verification
CC=${CC:-mips64r5900el-ps2-elf-gcc}
NM=${NM:-mips64r5900el-ps2-elf-nm}
OBJDUMP=${OBJDUMP:-mips64r5900el-ps2-elf-objdump}
# Existing feature-hook setting; override with ENABLE_FEATURE_HOOKS=0 if desired.
ENABLE_FEATURE_HOOKS=${ENABLE_FEATURE_HOOKS:-1}
extra=()
if [[ "$ENABLE_FEATURE_HOOKS" == 1 ]]; then extra+=(--features); fi
if [[ -n "$GAME_ELF" ]]; then
    extra+=(--game "$GAME_ELF")
    python3 GENPnach.py "${extra[@]}"
fi
mkdir -p bin
flags=(-O2 -std=gnu11 -G0 -mno-abicalls -fno-pic -ffreestanding -fno-builtin
 -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fomit-frame-pointer
 '-ffixed-$16' '-ffixed-$17' '-ffixed-$18' '-ffixed-$19' '-ffixed-$20'
 '-ffixed-$21' '-ffixed-$22' '-ffixed-$23' '-ffixed-$28' '-ffixed-$30')
for number in {20..31}; do flags+=("-ffixed-\$f${number}"); done
"$CC" "${flags[@]}" -c main.c -o bin/SOCOM.o
"$CC" "${flags[@]}" -c ui.c -o bin/ui.o
"$CC" -G0 -mno-abicalls -fno-pic -nostdlib -nostartfiles \
 -Wl,--build-id=none,-Map,bin/SOCOM.map -T linker/cave.ld bin/ui.o bin/SOCOM.o -o bin/SOCOM.elf
if [[ -n "$("$NM" -u bin/SOCOM.elf)" ]]; then echo 'Unresolved payload symbols'; exit 1; fi
"$OBJDUMP" -d bin/SOCOM.elf > bin/SOCOM.asm.txt
python3 GENPnach.py bin/SOCOM.elf bin/SOCOM.pnach --audit bin/SOCOM.asm.txt "${extra[@]}"
