#ifndef HOOK_H
#define HOOK_H

#include "memory.h"

typedef enum
{
    HOOK_TYPE_NONE = 0,
    HOOK_TYPE_CALL,         // redirect existing jal 
    HOOK_TYPE_JUMP,         // redirect existing j
    HOOK_TYPE_JUMP_RETURN,           // redirect existing jr
    HOOK_TYPE_FUNCTION,     // detour function head and trampoline to original
} HOOK_TYPE;

typedef struct
{
    HOOK_TYPE type;
    u32 address;
    u32 target;
    u32 original[2];
    void* trampoline;
    bool installed;
} Hook;


void Hook_Init(Hook* hook, HOOK_TYPE type, u32 address, u32 target);
bool Hook_Create(Hook* hook);
bool Hook_Remove(Hook* hook);
bool Hook_IsCreated(const Hook* hook);
void* Hook_GetOriginal(const Hook* hook);

#define DefineHookFor(name) static Hook name##_Hook; static name##_t name##_Original = NULL
#define DefineHook(name, ret, params, addr) DefineFunction(name, ret, params, addr); DefineHookFor(name)
#define CreateDetour(name, replacement) do { Hook_Init(&name##_Hook, HOOK_TYPE_FUNCTION, (u32)(name), (u32)(replacement)); if (Hook_Create(&name##_Hook)) name##_Original = (name##_t)Hook_GetOriginal(&name##_Hook); } while (0)
#define RemoveDetour(name) do { if (Hook_Remove(&name##_Hook)) name##_Original = NULL; } while (0)
#define CreateCall(hook, address, replacement) (Hook_Init(&(hook), HOOK_TYPE_CALL, (u32)(address), (u32)(replacement)), Hook_Create(&(hook)))
#define RemoveCall(hook) Hook_Remove(&(hook))
#define CreateJump(hook, address, replacement) (Hook_Init(&(hook), HOOK_TYPE_JUMP, (u32)(address), (u32)(replacement)), Hook_Create(&(hook)))
#define RemoveJump(hook) Hook_Remove(&(hook))
#define CreateJr(hook, address, replacement) (Hook_Init(&(hook), HOOK_TYPE_JUMP_RETURN, (u32)(address), (u32)(replacement)), Hook_Create(&(hook)))
#define RemoveJr(hook) Hook_Remove(&(hook))

#endif // HOOK_H
