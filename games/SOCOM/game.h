#ifndef SOCOM_GAME_H
#define SOCOM_GAME_H

#include "structs.h"

#define ELF_PATH "cdrom0:\\SCUS_971.34;1"


// ------------------------------------------------------------
// Globals
// ------------------------------------------------------------
#define gHud 0x48E594
#define gWorld 0x48D848
#define gCamera 0x51E778
#define gSealArray 0x4D46A0
#define gPickupArray 0x51E970
#define gAppCamera 0x48D488
#define tickAnimFireWeapon 0x1B7C90
#define g_GS_Z_OFFSET 0x0048CF50
#define g_theMission 0x4D4880

#define g_MessageQue 0x4D4990

#define g_vft_BatchRelocator 0x48A520
#define g_vft_2DString_2 0x48A730
#define g_CInput_pads 0x48C8D0

#define g_theNetwork_2 0x52A5A9
#define g_tagFirepoint_default 0x46B500
#define g_tagFirepoint_203 0x46B4F0
#define g_net_isHost 0x52A5A8


// ------------------------------------------------------------
// PATCHES
// ------------------------------------------------------------
#define fn_MissionTick_BEQ_MISSION_SUCCESS 0x1F90F0 
#define AUTO_COMPLETE_ORIGINAL   0x10400005u  // beqz v0, loc_1F9108
#define AUTO_COMPLETE_PATCHED    0x14400005u  // bnez v0, loc_1F9108 ; force the branch event to OnMissionComplete 

#define fn_ToggleReady_JR_RA_FORCE_START 0x20B81C
#define FORCE_START_ORIGINAL     0x03E00008u  // jr ra
#define FORCE_START_PATCHED      0x08075368u  // j 0x1D4DA0 ; jump to UIForceMPLaunch @ 0x1D4DA0 lol

#define fn_MissionTick_JAL_MP_ROUND_END 0x1F90F8
#define NEVER_END_ORIGINAL       0x0C07E160u  // jal Mission::OnMissionComplete
#define NEVER_END_PATCHED        0x00000000u  // nop


// ------------------------------------------------------------
// SYSCALLS
// ------------------------------------------------------------
typedef s32 (*FlushCache_t)(s32 mode);
#define FlushCache ((FlushCache_t)0x0015AB60)

// ------------------------------------------------------------
// Native functions
// ------------------------------------------------------------

//
// ============================================================================
//  PLAYER / ENTITY
// ============================================================================
//

typedef CZSealBody* (*ftsGetPlayer_t)(void);
#define ftsGetPlayer ((ftsGetPlayer_t)0x00200010)

typedef void (*ftsSetPlayer_t)(CZSealBody* player);
#define ftsSetPlayer ((ftsSetPlayer_t)0x00200020)

typedef void (*CEntity_SetTeamMask_t)(CEntity* entity, s32 teamMask);
#define CEntity_SetTeamMask ((CEntity_SetTeamMask_t)0x001F00B0)

typedef void (*CEntity_JoinTeam_t)(CEntity* entity, u32 team);
#define CEntity_JoinTeam ((CEntity_JoinTeam_t)0x001F0080)

typedef u32 (*ClearTeamMask_t)(u32 mask, u32 team);
#define ClearTeamMask ((ClearTeamMask_t)0x002FF660)

typedef bool (*CNode_Rendered_t)(CNode* node);
#define CNode_Rendered ((CNode_Rendered_t)0x0022A4B0)


//
// ============================================================================
//  SEAL BODY / CHARACTER
// ============================================================================
//

typedef s64 (*GetCharacterHit_t)(CZSealBody* seal, f32* outOrigin, s64 a3);
#define GetCharacterHit ((GetCharacterHit_t)0x002A8200)

typedef void (*CZSealBody_CheckDIShoot_t)(CZSealBody* seal, s64 a2, s32 a3);
#define CZSealBody_CheckDIShoot ((CZSealBody_CheckDIShoot_t)0x002A7490)

typedef bool (*CZSealBody_GetFirepointPos_t)(CZSealBody* seal, f32* outPosition, s64 a3);
#define CZSealBody_GetFirepointPos ((CZSealBody_GetFirepointPos_t)0x002D1720)

typedef s64 (*CZSealBody_TeleportTo_t)(CZSealBody* seal, const Matrix4x4* matrix);
#define CZSealBody_TeleportTo ((CZSealBody_TeleportTo_t)0x002537D0)


//
// ============================================================================
//  SEAL UNITS / TEAMS
// ============================================================================
//

typedef CSealUnit* (*CSealUnit_GetUnitByTeam_t)(u32 team);
#define CSealUnit_GetUnitByTeam ((CSealUnit_GetUnitByTeam_t)0x002EABC0)

typedef u64 (*CSealUnit_SealJoinUnit_t)(CSealUnit* unit, CZSealBody* seal);
#define CSealUnit_SealJoinUnit ((CSealUnit_SealJoinUnit_t)0x002E9EF0)


//
// ============================================================================
//  WEAPONS / KIT
// ============================================================================
//

typedef bool (*CZKit_IsLauncherWeapon_t)(CZWeapon* weapon);
#define CZKit_IsLauncherWeapon ((CZKit_IsLauncherWeapon_t)0x0032EB90)

typedef bool (*CZKit_WeaponIsGrenadeLauncherMode_t)(CZKit* kit, s64 a2);
#define CZKit_WeaponIsGrenadeLauncherMode ((CZKit_WeaponIsGrenadeLauncherMode_t)0x002B6710)

typedef void (*CZKit_HandleFireWeapon_t)(CZKit* kit, s64 a2, s64 a3, f32 a4);
#define CZKit_HandleFireWeapon ((CZKit_HandleFireWeapon_t)0x002B7730)

// Currently unused.
typedef void (*CZKit_WillFireWeapon_t)(CZKit* kit, f32 a2);
#define CZKit_WillFireWeapon ((CZKit_WillFireWeapon_t)0x002B9140)


//
// ============================================================================
//  2D STRING / FONT RENDERING
// ============================================================================
//

// Glyph lookup used by retail C2DString::MakePacket.
typedef u32 (*C2DFont_GetEntry_t)(u32 font, s32 character);
#define C2DFont_GetEntry ((C2DFont_GetEntry_t)0x00316130)

// Loads a string with the specified font and screen position.
typedef void (*C2DString_Load_t)(C2DString* self, const char* text, C2DFont* font, s32 x, s32 y);
#define C2DString_Load ((C2DString_Load_t)0x00319340)

// Draws the string using the specified camera.
typedef void (*C2DString_Draw_t)(C2DString* self, CZCamera* camera);
#define C2DString_Draw ((C2DString_Draw_t)0x003196D0)

typedef void (*C2DString_MakePacket_t)(void* self, const u32* relocator, s32 depth);
#define C2DString_MakePacket ((C2DString_MakePacket_t)0x00319750)

typedef void (*C2DPoly_MakePacket_t)(void* self, void* a2);
#define C2DPoly_MakePacket ((C2DPoly_MakePacket_t)0x003180F0)


//
// ============================================================================
//  LINE / DEBUG RENDERING
// ============================================================================
//

// Draws a line from the previous point to the supplied point.
typedef void (*AI_LineTo_t)(f32* point);
#define AI_LineTo ((AI_LineTo_t)0x002F81A0)

// Draws a line in 3D space.
typedef void (*AI_DrawLine_t)(u64 mask, f32* start, f32* end);
#define AI_DrawLine ((AI_DrawLine_t)0x002F8030)

// Draws a line in world space.
typedef void (*RenderLineWorld_t)(f32* start, f32* end, f32* colorStart, f32* colorEnd);
#define RenderLineWorld ((RenderLineWorld_t)0x00249590)

// Draws a line in screen/canvas space.
typedef void (*Draw2DLine_t)(f32* start, f32* end, f32* colorStart, f32* colorEnd);
#define Draw2DLine ((Draw2DLine_t)0x00316FB0)

// Draws a line in world space.
typedef void (*Draw3DLine_t)(f32* start, f32* end, f32* colorStart, f32* colorEnd);
#define Draw3DLine ((Draw3DLine_t)0x00317220)

typedef bool (*Clip3DLine_t)(s64 a1, s64 a2, s64 a3);
#define Clip3DLine ((Clip3DLine_t)0x00316C90)


//
// ============================================================================
//  MESSAGE RENDERING
// ============================================================================
//

// Adds a message to the top-center message queue.
typedef s64 (*C2DMessage_AddMessage_t)(void* queue, const char* message, f32 a3);
#define C2DMessage_AddMessage ((C2DMessage_AddMessage_t)0x00202C70)


//
// ============================================================================
//  VU0 / MATRIX / VECTOR
// ============================================================================
//

// Copies a matrix from src to dst.
typedef void (*sceVu0CopyMatrix_t)(s64 dst, s64 src);
#define sceVu0CopyMatrix ((sceVu0CopyMatrix_t)0x001518A8)

typedef void (*sceVu0CopyVector_t)(void* dst, void* src);
#define sceVu0CopyVector ((sceVu0CopyVector_t)0x00151898)

typedef void (*sceVu0FTOI0Vector_t)(s32* dst, f32* src);
#define sceVu0FTOI0Vector ((sceVu0FTOI0Vector_t)0x001518E0)

typedef void (*sceVu0RotTransPers_t)(s32* dst,f32* matrix,f32* src,s32 mode);
#define sceVu0RotTransPers ((sceVu0RotTransPers_t)0x001520E8)


//
// ============================================================================
//  GS / PACKET / VIDEO
// ============================================================================
//

typedef u64 (*zSysSprGetPacket_FPP1_t)(s32 a1);
#define zSysSprGetPacket_FPP1 ((zSysSprGetPacket_FPP1_t)0x0030D1E0)

typedef void (*zSysFifoKick_t)(void* packet, s32 a2);
#define zSysFifoKick ((zSysFifoKick_t)0x0030CFB0)

// Useful render hook point.
typedef void (*zVid_ZTestOn_t)(void);
#define zVid_ZTestOn ((zVid_ZTestOn_t)0x0031ED90)


//
// ============================================================================
//  GAME / HUD / INPUT
// ============================================================================
//

// Useful input hook point.
typedef u64 (*recoTick_t)(void);
#define recoTick ((recoTick_t)0x00352B70)

// Useful pause/game-state hook point.
typedef u64 (*CHUD_PauseGame_t)();
#define CHUD_PauseGame ((CHUD_PauseGame_t)0x003D9DF0)


// ------------------------------------------------------------
// Constants
// ------------------------------------------------------------

char* launch_cmd[] =
{
    "-m",
};

char* load_arg[] =
{
    "dlgExitState.rdr",
    "dlgGameLobby.rdr",
    "dlgIntroScreen.rdr",
    "dlgLoad.rdr",
    "dlgMultiplayerFinal.rdr",  
    "dlgMultiplayerRound.rdr",
    "dlgNetAbort.rdr",
    "dlgNetError.rdr"
};

char* launch_arg[] =
{
    "dlgAfterErrorReboot.rdr",
    "dlgAfterReboot.rdr",
    "dlgExitState.rdr",     // -- black screen
    "dlgIntroScreen.rdr",   // -- normal launch
    "dlgLoad.rdr",          // -- black screen "press triangle button to return to the lobby"
    "dlgMultiplayerFinal.rdr",  
    "dlgMultiplayerRound.rdr",
    "dlgNetAbandoned.rdr",
    "dlgNetAbort.rdr",
    "dlgNetError.rdr",
    "dlgReturnFromNTGUI2.rdr" // -- launches to mulitplayer menu 
};

char* launch_flag[] =
{
    "--none",
};




#endif