#include "Memory.h"
#include "Game.h"

u8 Memory_ReadU8(u32 address) { return *(volatile u8*)address; }

u16 Memory_ReadU16(u32 address) { return *(volatile u16*)address; }

u32 Memory_ReadU32(u32 address) { return *(volatile u32*)address; }

f32 Memory_ReadFloat(u32 address) { return *(volatile f32*)address; }

void Memory_WriteU8(u32 address, u8 value) { *(volatile u8*)address = value; }

void Memory_WriteU16(u32 address, u16 value) { *(volatile u16*)address = value; }

void Memory_WriteU32(u32 address, u32 value) { *(volatile u32*)address = value; }

void Memory_WriteFloat(u32 address, f32 value) { *(volatile f32*)address = value; }

u32 Memory_EncodeJump(u32 target) { return 0x08000000u | ((target >> 2) & 0x03FFFFFFu); }

u32 Memory_EncodeCall(u32 target) { return 0x0C000000u | ((target >> 2) & 0x03FFFFFFu); }

void Memory_PatchInstruction(u32 address, u32 instruction)
{
    Memory_WriteU32(address, instruction);

    Memory_FlushCache();
}

bool Memory_PatchInstructionChecked(u32 address, u32 expected, u32 replacement)
{
    if (Memory_ReadU32(address) != expected)
        return false;

    Memory_WriteU32(address, replacement);

    Memory_FlushCache();

    return true;
}

void Memory_PatchInstructions(u32 address, const u32* instructions, u32 count)
{
    volatile u32* dst = (volatile u32*)address;

    for (u32 i = 0; i < count; i++)
        dst[i] = instructions[i];

    Memory_FlushCache();
}

void Memory_MakeNOP(u32 address)
{
    Memory_PatchInstruction(address, 0x00000000u);
}

void Memory_MakeJump(u32 address, u32 target)
{
    Memory_PatchInstruction(address, Memory_EncodeJump(target));
}

void Memory_MakeCall(u32 address, u32 target)
{
    Memory_PatchInstruction(address, Memory_EncodeCall(target));
}

static MemoryFlushCacheFn m_flushCache = NULL;

void Memory_SetFlushCacheFunction(MemoryFlushCacheFn function)
{
    m_flushCache = function;
}

void Memory_FlushCache(void)
{
    if (!m_flushCache)
        return;

    m_flushCache(0);
    m_flushCache(2);
}
