#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"

//
// ============================================================================
//  MEMORY ACCESS
// ============================================================================
//

u8  Memory_ReadU8(u32 address);
u16 Memory_ReadU16(u32 address);
u32 Memory_ReadU32(u32 address);
f32 Memory_ReadFloat(u32 address);

void Memory_WriteU8(u32 address, u8 value);
void Memory_WriteU16(u32 address, u16 value);
void Memory_WriteU32(u32 address, u32 value);
void Memory_WriteFloat(u32 address, f32 value);

//
// ============================================================================
//  REGISTERS
// ============================================================================
//

u32 Memory_GetGP(void);
u32 Memory_GetSP(void);
u32 Memory_GetRA(void);

//
// ============================================================================
//  INSTRUCTION ENCODING
// ============================================================================
//

u32 Memory_EncodeJump(u32 target);
u32 Memory_EncodeCall(u32 target);

//
// ============================================================================
//  INSTRUCTION PATCHING
// ============================================================================
//

void Memory_PatchInstruction(u32 address, u32 instruction);
bool Memory_PatchInstructionChecked(u32 address, u32 expected, u32 replacement);
void Memory_PatchInstructions(u32 address, const u32* instructions, u32 count);

void Memory_MakeNOP(u32 address);
void Memory_MakeJump(u32 address, u32 target);
void Memory_MakeCall(u32 address, u32 target);

//
// ============================================================================
//  CACHE
// ============================================================================
//

typedef s32 (*MemoryFlushCacheFn)(s32 mode);

void Memory_SetFlushCacheFunction(MemoryFlushCacheFn function);
void Memory_FlushCache(void);

#endif // MEMORY_H