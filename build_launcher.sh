#!/bin/bash

set -e

if [ -z "$PS2SDK" ]; then
    echo "Error: PS2SDK is not set. Run the PS2 toolchain environment script first."
    exit 1
fi

ROOT_DIR=$(cd "$(dirname "$0")" && pwd)

# build patch
echo "Building payload"
"$ROOT_DIR/build.sh"

# echo "Embedding payload as resource"
# @todo: embed payload into elf

echo "Building launcher ELF..."
make -C "$ROOT_DIR/launcher" clean
make -C "$ROOT_DIR/launcher"

echo
echo "Generated: $ROOT_DIR/bin/PS2-NativeHooks.elf"