#ifndef SOCOM_COMMON_H
#define SOCOM_COMMON_H

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
// Function Definitions
// ------------------------------------------------------------

#define DefineFunction(name, ret, params, addr) typedef ret (*name##_t) params; static const name##_t name = (name##_t)(addr)
#define CallFunction(ret, params, addr) ((ret (*) params)(addr))

#endif // SOCOM_COMMON_H
