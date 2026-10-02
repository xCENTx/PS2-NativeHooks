#include "hook.h"

#define HOOK_TRAMPOLINE_INSTRUCTIONS    4
#define HOOK_MAX_TRAMPOLINES            16

typedef struct
{
    u32 instructions[HOOK_TRAMPOLINE_INSTRUCTIONS];

} hkTrampoline;

__attribute__((section(".hooks"), aligned(16)))
static hkTrampoline m_hookTrampolines[HOOK_MAX_TRAMPOLINES];

__attribute__((section(".hooks")))
static u32 m_hookTrampolineCount = 0;

static hkTrampoline* Hook_AllocateTrampoline(void)
{
    if (m_hookTrampolineCount >= HOOK_MAX_TRAMPOLINES)
        return 0;

    hkTrampoline* trampoline = &m_hookTrampolines[m_hookTrampolineCount];

    m_hookTrampolineCount++;

    return trampoline;
}

void Hook_Init(Hook* hook, HOOK_TYPE type, u32 address, u32 target)
{
    if (!hook)
        return;

    hook->type = type;

    hook->address = address;
    hook->target  = target;

    hook->original[0] = 0;
    hook->original[1] = 0;

    hook->trampoline = 0;

    hook->installed = false;
}

bool Hook_Create(Hook* hook)
{
    if (!hook)
        return false;

    if (hook->installed)
        return true;


    switch (hook->type)
    {
        case HOOK_TYPE_CALL:
        {
            hook->original[0] = Memory_ReadU32(hook->address);

            Memory_MakeCall(hook->address, hook->target);

            break;
        }

        case HOOK_TYPE_JUMP:
        case HOOK_TYPE_JUMP_RETURN:
        {
            hook->original[0] = Memory_ReadU32(hook->address);

            Memory_MakeJump(hook->address, hook->target);

            break;
        }

        case HOOK_TYPE_FUNCTION:
        {
            hkTrampoline* trampoline = Hook_AllocateTrampoline();

            if (!trampoline)
                return false;


            // Save displaced instructions.

            hook->original[0] = Memory_ReadU32(hook->address + 0);

            hook->original[1] = Memory_ReadU32(hook->address + 4);


            // Build trampoline.

            trampoline->instructions[0] = hook->original[0];

            trampoline->instructions[1] = hook->original[1];

            trampoline->instructions[2] = Memory_EncodeJump(hook->address + 8);

            trampoline->instructions[3] = 0x00000000u;


            // Install function detour.

            Memory_WriteU32(hook->address + 0, Memory_EncodeJump(hook->target));

            Memory_WriteU32(hook->address + 4, 0x00000000u);

            Memory_FlushCache();


            hook->trampoline = (void*)trampoline;

            break;
        }


        default:
            return false;
    }


    hook->installed = true;

    return true;
}

bool Hook_Remove(Hook* hook)
{
    if (!hook)
        return false;

    if (!hook->installed)
        return true;


    switch (hook->type)
    {
        case HOOK_TYPE_CALL:
        case HOOK_TYPE_JUMP:
        {
            Memory_PatchInstruction(hook->address, hook->original[0]);

            break;
        }
        
        case HOOK_TYPE_FUNCTION:
        {
            Memory_WriteU32(hook->address + 0, hook->original[0]);

            Memory_WriteU32(hook->address + 4, hook->original[1]);

            Memory_FlushCache();

            break;
        }


        default:
            return false;
    }


    hook->installed = false;

    return true;
}

bool Hook_IsCreated(const Hook* hook)
{
    if (!hook)
        return false;

    return hook->installed;
}

void* Hook_GetOriginal(const Hook* hook)
{
    if (!hook)
        return 0;

    if (hook->type != HOOK_TYPE_FUNCTION)
        return 0;

    if (!hook->installed)
        return 0;

    return hook->trampoline;
}
