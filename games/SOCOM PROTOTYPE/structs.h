// structs.h
#ifndef SOCOM_STRUCTS_H
#define SOCOM_STRUCTS_H

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
// Forward declarations
// ------------------------------------------------------------

// ------------------------------------------------------------
// Enums
// ------------------------------------------------------------
typedef s8 RDR_COMMANDS;
enum
{
    RDR_EXIT,
    RDR_GAME_LOBBY,
    RDR_INTRO,
    RDR_LOAD,
    RDR_MP_FINAL,
    RDR_MP_ROUND,
    RDR_NET_ABORT,
    RDR_NET_ERROR,
};

typedef s8 LAUNCH_COMMANDS;
enum
{
    LAUNCH_CMD_d,   // set database
    LAUNCH_CMD_p,   // CFileIO::SetRootPath
    LAUNCH_CMD_m,   // main
    LAUNCH_CMD_V,   // attempts to set frame rate
    LAUNCH_CMD_h,   // toggles gopt byte 0x00
    LAUNCH_CMD_f,   // toggles gopt byte 0x01
    LAUNCH_CMD_F,   // toggles gopt byte 0x02
    LAUNCH_CMD_R,   // toggles gopt byte 0x03
    LAUNCH_CMD_A,   // toggles gopt byte 0x05
    LAUNCH_CMD_a,   // toggles gopt byte 0x06
    LAUNCH_CMD_s,   // toggles character shadows
    LAUNCH_CMD_z,   // disables ZAR animation loading
    LAUNCH_CMD_Z,   // enables ZAR animation loading
};

typedef s8 LAUNCH_FLAGS;
enum
{
    LAUNCH_FLAGS_AP,
    LAUNCH_FLAGS_AMMO,
};

// ------------------------------------------------------------
// Structs
// ------------------------------------------------------------


#endif
