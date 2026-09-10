import struct
import sys

HOOK_ADDR = 0x001EBF20
CODE_CAVE = 0x00097000

if len(sys.argv) != 5:
    print(
        f"Usage: python3 {sys.argv[0]} "
        "<hook_address> <code_cave_address> <input.bin> <output.pnach>"
    )
    sys.exit(1)

HOOK_ADDR = int(sys.argv[1], 0)
CODE_CAVE = int(sys.argv[2], 0)
INPUT_FILE = sys.argv[3]
OUTPUT_FILE = sys.argv[4]

if CODE_CAVE & 3:
    print("Error: code cave address must be 4-byte aligned.")
    sys.exit(1)

with open(INPUT_FILE, "rb") as f:
    data = f.read()

if len(data) & 3:
    print("Error: payload size must be 4-byte aligned.")
    sys.exit(1)

# Calculate JAL to our code cave.
jal_opcode = (
    0x0C000000 |
    ((CODE_CAVE >> 2) & 0x03FFFFFF)
)

with open(OUTPUT_FILE, "w") as out:

    # Replace original call with JAL to our code cave.
    pnach_addr = 0x20000000 | HOOK_ADDR

    out.write(
        f"patch=1,EE,{pnach_addr:08X},extended,{jal_opcode:08X}\n"
    )

    # Write the linked payload into the code cave.
    for offset in range(0, len(data), 4):

        value = struct.unpack_from("<I", data, offset)[0]

        address = CODE_CAVE + offset
        pnach_addr = 0x20000000 | address

        out.write(
            f"patch=1,EE,{pnach_addr:08X},extended,{value:08X}\n"
        )

print(f"Generated {OUTPUT_FILE}")
print(f"Payload size: 0x{len(data):X} bytes")
print(
    f"Payload range: "
    f"0x{CODE_CAVE:08X} - "
    f"0x{CODE_CAVE + len(data) - 1:08X}"
)