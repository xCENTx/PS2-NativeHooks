#ifndef SOCOM_COMMON_H
#define SOCOM_COMMON_H

#include <assert.h>
#include <stddef.h>
#include <stdbool.h>


// ------------------------------------------------------------
// Primitive types
// ------------------------------------------------------------

typedef signed char        s8;
typedef unsigned char      u8;

typedef signed short       s16;
typedef unsigned short     u16;

typedef signed int         s32;
typedef unsigned int       u32;

typedef signed long long   s64;
typedef unsigned long long u64;

typedef float              f32;



// ------------------------------------------------------------
// Core Structures (temporary storage)
// ------------------------------------------------------------

typedef struct
{
    s32 status;             // 0x00
    void* func;             // 0x04
    void* stack;            // 0x08
    s32 stack_size;         // 0x0C
    void* gp_reg;           // 0x10
    s32 initial_priority;   // 0x14
    s32 current_priority;   // 0x18
    u32 attr;               // 0x1C
    u32 option;             // 0x20
} ee_thread_t;
_Static_assert(sizeof(ee_thread_t) == 0x24, "ee_thread_t size mismatch");

// ------------------------------------------------------------
// Function Definitions
// ------------------------------------------------------------

#define DefineFunction(name, ret, params, addr) typedef ret (*name##_t) params; static const name##_t name = (name##_t)(addr)
#define CallFunction(ret, params, addr) ((ret (*) params)(addr))

#endif // SOCOM_COMMON_H
