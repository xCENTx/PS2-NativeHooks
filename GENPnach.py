import struct
import sys
import argparse


# Number of bools in CHEAT_FEATURES / CHEAT_MAX.
# sizeof(bool) == 1 with this toolchain.
FEATURE_COUNT = 6


def make_jal(address):
    """
    Encode a MIPS JAL instruction for the supplied target address.
    """
    if address & 3:
        raise ValueError(
            f"JAL target 0x{address:08X} is not 4-byte aligned."
        )

    return (
        0x0C000000 |
        ((address >> 2) & 0x03FFFFFF)
    )


def write_patch(out, address, value, mode=1):
    """
    Write a 32-bit EE PNACH patch.

    mode=1 -> continuously applied
    mode=0 -> applied once
    """
    pnach_addr = 0x20000000 | address

    out.write(
        f"patch={mode},EE,{pnach_addr:08X},extended,{value:08X}\n"
    )


def overlaps_feature_array(address, feature_addr, feature_size):
    """
    Returns True if the 4-byte payload write beginning at 'address'
    overlaps any byte belonging to m_featureset.
    """
    write_start = address
    write_end = address + 4

    feature_start = feature_addr
    feature_end = feature_addr + feature_size

    return (
        write_start < feature_end and
        write_end > feature_start
    )


if len(sys.argv) < 11:
    print(
        f"Usage: python3 {sys.argv[0]} "
        "<code_cave> "
        "<input.bin> "
        "<output.pnach> "
        "<featureset_address> "
        "<hook1_address> <hook1_target> "
        "<hook2_address> <hook2_target>"
        "<hook3_address> <hook3_target>"
    )
    sys.exit(1)


CODE_CAVE = int(sys.argv[1], 0)
INPUT_FILE = sys.argv[2]
OUTPUT_FILE = sys.argv[3]

FEATURESET_ADDR = int(sys.argv[4], 0)

CHECKDISHOOT_HOOK_ADDR = int(sys.argv[5], 0)
CHECKDISHOOT_TARGET = int(sys.argv[6], 0)

HANDLEFIREWEAPON_HOOK_ADDR = int(sys.argv[7], 0)
HANDLEFIREWEAPON_TARGET = int(sys.argv[8], 0)

TOGGLEREADY_HOOK_ADDR = int(sys.argv[9], 0)
TOGGLEREADY_TARGET = int(sys.argv[10], 0)


# Optional additions: original eight positional arguments still work unchanged.
parser = argparse.ArgumentParser(add_help=False)
parser.add_argument('--mutable-range', nargs=2, type=lambda value: int(value, 0))
parser.add_argument('--hook', nargs=2, action='append', default=[], type=lambda value: int(value, 0))
parser.add_argument('--restore', nargs=2, action='append', default=[], type=lambda value: int(value, 0))
extra = parser.parse_args(sys.argv[11:])


#
# Sanity checks.
#
if CODE_CAVE & 3:
    print("Error: code cave address must be 4-byte aligned.")
    sys.exit(1)


#
# Read linked payload.
#
with open(INPUT_FILE, "rb") as f:
    data = f.read()


if len(data) & 3:
    print("Error: payload size must be 4-byte aligned.")
    sys.exit(1)


payload_end = CODE_CAVE + len(data)

if not (
    CODE_CAVE <= FEATURESET_ADDR < payload_end
):
    print(
        "Error: m_featureset is outside the extracted payload.\n"
        f"  m_featureset: 0x{FEATURESET_ADDR:08X}\n"
        f"  payload:      0x{CODE_CAVE:08X} - "
        f"0x{payload_end - 1:08X}"
    )
    sys.exit(1)


if extra.mutable_range:
    start, end = extra.mutable_range
    if start & 3 or end & 3 or not CODE_CAVE <= start <= end <= payload_end:
        sys.exit('Error: mutable range must be aligned and inside the binary.')
for site, target in extra.hook:
    if site & 3 or not CODE_CAVE <= target < payload_end:
        sys.exit('Error: extra hook site/target is invalid.')
    make_jal(target)


#
# Generate JAL instructions.
#
try:
    checkdishoot_jal = make_jal(
        CHECKDISHOOT_TARGET
    )

    handlefireweapon_jal = make_jal(
        HANDLEFIREWEAPON_TARGET
    )

    toggleready_jal = make_jal(
        TOGGLEREADY_TARGET
    )

except ValueError as e:
    print(f"Error: {e}")
    sys.exit(1)


with open(OUTPUT_FILE, "w") as out:

    #
    # Disable
    #
    out.write(
        "[NATIVE-MENU\\DISABLE]\n"
        "author=NightFyre\n"
        "description=clean up\n"
    )

    #
    # Restore original game instructions.
    #

    # CCameraApp::Tick -> CZSealBody::CheckDIShoot
    write_patch( out, CHECKDISHOOT_HOOK_ADDR, 0x0C0A9D24 )

    # CZKit -> HandleFireWeapon
    write_patch( out, HANDLEFIREWEAPON_HOOK_ADDR, make_jal(0x002B7730) )

    # CZPersonaState -> ToggleReady
    write_patch( out, TOGGLEREADY_HOOK_ADDR, 0x0C082DF4 )

    for site, original_word in extra.restore:
        write_patch(out, site, original_word)

    #
    # Enable
    #
    out.write(
        "[NATIVE-MENU\\ENABLE]\n"
        "author=NightFyre\n"
        "description=L3 + R3 to show / hide menu\n"
    )

    #
    # CCameraApp::Tick
    #
    # Replace original call with:
    #
    #     jal hk_CheckDIShoot
    #
    write_patch(
        out,
        CHECKDISHOOT_HOOK_ADDR,
        checkdishoot_jal
    )

    #
    # CZKit hook
    #
    # Replace original call with:
    #
    #     jal hk_HandleFireWeapon
    #
    write_patch(
        out,
        HANDLEFIREWEAPON_HOOK_ADDR,
        handlefireweapon_jal
    )

    #
    # CZPersona hook
    #
    # Replace original call with:
    #
    #     jal hk_ToggleReady
    #
    write_patch(
        out,
        TOGGLEREADY_HOOK_ADDR,
        toggleready_jal
    )

    for site, target in extra.hook:
        write_patch(out, site, make_jal(target))

    #
    # Write linked payload into the code cave.
    #
    # Everything is patch=1 EXCEPT DWORDs which overlap
    # m_featureset. Those use patch=0 so PCSX2 initializes
    # them once and our runtime code can modify the bools
    # without PNACH continuously resetting them.
    #
    for offset in range(0, len(data), 4):

        value = struct.unpack_from(
            "<I",
            data,
            offset
        )[0]

        address = CODE_CAVE + offset

        if overlaps_feature_array(
            address,
            FEATURESET_ADDR,
            FEATURE_COUNT
        ):
            mode = 0
        else:
            mode = 1

        if extra.mutable_range and overlaps_feature_array(
            address, extra.mutable_range[0], extra.mutable_range[1] - extra.mutable_range[0]
        ):
            mode = 0

        write_patch(out, address, value, mode)


print(f"Generated {OUTPUT_FILE}")
print()

print("Hooks:")

print(
    f"  CheckDIShoot:\n"
    f"    0x{CHECKDISHOOT_HOOK_ADDR:08X}"
    f" -> 0x{CHECKDISHOOT_TARGET:08X}"
    f"  JAL={checkdishoot_jal:08X}"
)

print(
    f"  HandleFireWeapon:\n"
    f"    0x{HANDLEFIREWEAPON_HOOK_ADDR:08X}"
    f" -> 0x{HANDLEFIREWEAPON_TARGET:08X}"
    f"  JAL={handlefireweapon_jal:08X}"
)

print(
    f"  ToggleReady:\n"
    f"    0x{TOGGLEREADY_HOOK_ADDR:08X}"
    f" -> 0x{TOGGLEREADY_TARGET:08X}"
    f"  JAL={toggleready_jal:08X}"
)

print()

print("Feature state:")
print(
    f"  m_featureset = 0x{FEATURESET_ADDR:08X}"
)

for i in range(FEATURE_COUNT):
    print(
        f"    [{i}] = 0x{FEATURESET_ADDR + i:08X}"
    )

print()

print(f"Payload size: 0x{len(data):X} bytes")

print(
    f"Payload range: "
    f"0x{CODE_CAVE:08X} - "
    f"0x{CODE_CAVE + len(data) - 1:08X}"
)