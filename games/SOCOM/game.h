#ifndef SOCOM_GAME_H
#define SOCOM_GAME_H

#include "structs.h"

#define ELF_PATH "cdrom0:\\SCUS_971.34;1"

// ------------------------------------------------------------
// Globals
// ------------------------------------------------------------

#define gConsole                    0x004B4630
#define gHud                        0x0048E594
#define gWorld                      0x0048D848
#define gCamera                     0x0051E778
#define gSealArray                  0x004D46A0
#define gPickupArray                0x0051E970
#define gAppCamera                  0x0048D488
#define gNetwork                    0x0052A5A0
#define tickAnimFireWeapon          0x001B7C90
#define g_GS_Z_OFFSET               0x0048CF50
#define g_theMission                0x004D4880
#define g_MessageQue                0x004D4990
#define g_vft_BatchRelocator        0x0048A520
#define g_vft_2DString_2            0x0048A730
#define g_CInput_pads               0x0048C8D0
#define g_theNetwork_2              0x0052A5A9
#define g_tagFirepoint_default      0x0046B500
#define g_tagFirepoint_203          0x0046B4F0
#define g_net_isHost                0x0052A5A8
#define g_gameVersion_1             0x00455C48
#define g_gameVersion_2             0x005BE6A0


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

DefineFunction(FlushCache, s32, (s32 mode), 0x0015AB60);
DefineFunction(CreateThread, s32, (ee_thread_t* thread), 0x0015A6E0);
DefineFunction(DeleteThread, s32, (s32 thread_id), 0x0015A6F0);
DefineFunction(StartThread, s32, (s32 thread_id, void* args), 0x0015A700);
DefineFunction(ExitThread, void, (void), 0x0015A710);
DefineFunction(ExitDeleteThread, void, (void), 0x0015A720);
DefineFunction(TerminateThread, s32, (s32 thread_id), 0x0015A730);
DefineFunction(ChangeThreadPriority, s32, (s32 thread_id, s32 priority), 0x0015A770);
DefineFunction(GetThreadId, s32, (void), 0x0015A7D0);
DefineFunction(SleepThread, s32, (void), 0x0015A800);
DefineFunction(WakeupThread, s32, (s32 thread_id), 0x0015A810);
DefineFunction(SuspendThread, s32, (s32 thread_id), 0x0015A850);
DefineFunction(ResumeThread, s32, (s32 thread_id), 0x0015A870);
DefineFunction(mcDelayThread, s32, (u16 delay), 0x00171278);

typedef s32 (*AlarmCallback_t)(s32 alarm_id, u16 time, void* common);

// ------------------------------------------------------------
// Native functions
// ------------------------------------------------------------

//
// ============================================================================
//  NODE
// ============================================================================
//

DefineFunction(CNode_Rendered, bool, (CNode* node), 0x0022A4B0);
DefineFunction(CNode_SetName, void, (CNode* node, const char* name), 0x00229C50);



//
// ============================================================================
//  CAMERA
// ============================================================================
//

DefineFunction(CAppCamera_Tick, void, (CAppCamera* camera), 0x0022B000);


//
// ============================================================================
//  PLAYER / ENTITY
// ============================================================================
//

DefineFunction(ftsGetPlayer, CZSealBody*, (void), 0x00200010);
DefineFunction(ftsSetPlayer, void, (CZSealBody* player), 0x00200020);
DefineFunction(ftsCreateSeal, CZSealBody*, (CCharacterType* character, const char* name, SEAL_CONTROL_TYPE controlType), 0x00200470);

DefineFunction(CEntity_JoinTeam, void, (CEntity* entity, u32 team), 0x001F0080);
DefineFunction(CEntity_SetTeamMask, void, (CEntity* entity, s32 teamMask), 0x001F00B0);
DefineFunction(CEntity_SetDisplayName, void, (CEntity* entity, const char* name), 0x001F0010);
DefineFunction(ClearTeamMask, u32, (u32 mask, u32 team), 0x002FF660);


//
// ============================================================================
//  SEAL BODY / CHARACTER
// ============================================================================
//

DefineFunction(GetCharacterHit, s64, (CZSealBody* seal, f32* outOrigin, s64 a3), 0x002A8200);
DefineFunction(CZSealBody_GetCharacter, CCharacterType*, (CZSealBody* seal), 0x002953F0);
DefineFunction(CZSealBody_CheckDIShoot, void, (CZSealBody* seal, s64 a2, s32 a3), 0x002A7490);
DefineFunction(CZSealBody_GetFirepointPos, bool, (CZSealBody* seal, f32* outPosition, s64 a3), 0x002D1720);
DefineFunction(CZSealBody_TeleportTo, s64, (CZSealBody* seal, const Matrix4x4* matrix), 0x002537D0);
DefineFunction(CZSealBody_Tick, void, (CZSealBody* seal, float deltaTime), 0x0025B160);

//
// ============================================================================
//  SEAL UNITS / TEAMS
// ============================================================================
//

DefineFunction(CSealUnit_InitalizeSealUnit, void, (CZSealBody* seal), 0x002E7820);
DefineFunction(CSealUnit_GetUnitByTeam, CSealUnit*, (u32 team), 0x002EABC0);
DefineFunction(CSealUnit_SealJoinUnit, u64, (CSealUnit* unit, CZSealBody* seal), 0x002E9EF0);


//
// ============================================================================
//  WEAPONS / KIT
// ============================================================================
//

DefineFunction(CZKit_IsLauncherWeapon, bool, (CZWeapon* weapon), 0x0032EB90);
DefineFunction(CZKit_WeaponIsGrenadeLauncherMode, bool, (CZKit* kit, s64 a2), 0x002B6710);
DefineFunction(CZKit_HandleFireWeapon, void, (CZKit* kit, s64 a2, s64 a3, f32 a4), 0x002B7730);
DefineFunction(CZKit_WillFireWeapon, void, (CZKit* kit, f32 a2), 0x002B9140);


//
// ============================================================================
//  2D STRING / FONT RENDERING
// ============================================================================
//

// Glyph lookup used by retail C2DString::MakePacket.
DefineFunction(C2DFont_GetEntry, u32, (u32 font, s32 character), 0x00316130);

// Loads a string with the specified font and screen position.
DefineFunction(C2DString_Load, void, (C2DString* self, const char* text, C2DFont* font, s32 x, s32 y), 0x00319340);

// Draws the string using the specified camera.
DefineFunction(C2DString_Draw, void, (C2DString* self, CZCamera* camera), 0x003196D0);

DefineFunction(C2DString_MakePacket, void, (void* self, const u32* relocator, s32 depth), 0x00319750);
DefineFunction(C2DPoly_MakePacket, void, (void* self, void* a2), 0x003180F0);


//
// ============================================================================
//  LINE / DEBUG RENDERING
// ============================================================================
//

// Draws a line from the previous point to the supplied point.
DefineFunction(AI_LineTo, void, (f32* point), 0x002F81A0);

// Draws a line in 3D space.
DefineFunction(AI_DrawLine, void, (u64 mask, f32* start, f32* end), 0x002F8030);

// Draws a line in world space.
DefineFunction(RenderLineWorld, void, (f32* start, f32* end, f32* colorStart, f32* colorEnd), 0x00249590);

// Draws a line in screen/canvas space.
DefineFunction(Draw2DLine, void, (f32* start, f32* end, f32* colorStart, f32* colorEnd), 0x00316FB0);

// Draws a line in world space.
DefineFunction(Draw3DLine, void, (f32* start, f32* end, f32* colorStart, f32* colorEnd), 0x00317220);

DefineFunction(Clip3DLine, bool, (s64 a1, s64 a2, s64 a3), 0x00316C90);


//
// ============================================================================
//  MESSAGE RENDERING
// ============================================================================
//

// Adds a message to the top-center message queue.
DefineFunction(C2DMessage_AddMessage, s64, (void* queue, const char* message, f32 a3), 0x00202C70);


//
// ============================================================================
//  VU0 / MATRIX / VECTOR
// ============================================================================
//

// Copies a matrix from src to dst.
DefineFunction(sceVu0CopyMatrix, void, (s64 dst, s64 src), 0x001518A8);
DefineFunction(sceVu0CopyVector, void, (void* dst, void* src), 0x00151898);
DefineFunction(sceVu0FTOI0Vector, void, (s32* dst, f32* src), 0x001518E0);
DefineFunction(sceVu0RotTransPers, void, (s32* dst, f32* matrix, f32* src, s32 mode), 0x001520E8);


//
// ============================================================================
//  GS / PACKET / VIDEO
// ============================================================================
//

DefineFunction(zSysSprGetPacket_FPP1, u64, (s32 a1), 0x0030D1E0);
DefineFunction(zSysFifoKick, void, (void* packet, s32 a2), 0x0030CFB0);

// Useful render hook point.
DefineFunction(zVid_ZTestOn, void, (void), 0x0031ED90);


//
// ============================================================================
//  GAME / HUD / INPUT
// ============================================================================
//

// Useful input hook point.
DefineFunction(recoTick, u64, (void), 0x00352B70);

// Useful pause/game-state hook point.
DefineFunction(CHUD_PauseGame, u64, (), 0x003D9DF0);


//
// ============================================================================
//  GAME / MISSION
// ============================================================================
//
DefineFunction(CMission_OnMissionComplete, void, (CMission* pMission, MISSION_STATE dwResult), 0x001F8580);


//
// ============================================================================
//  MULTIPLAYER / MLS / NETCODE
// ============================================================================
//

DefineFunction(CZNetwork_zNetUpdate, s64, (CZNetwork* network), 0x0033C910);

// executed whenever the player toggles ready - hook for force start
DefineFunction(CZPersonaState_ToggleReady, void, (void), 0x0020B7D0);

// force starts the lobby into prematch state with 10 second countdown
DefineFunction(UIForceMPLaunch, u64, (void), 0x001D4DA0);

// ------------------------------------------------------------
// Constants
// ------------------------------------------------------------

static char* launch_cmd[] =
{
    "-m",
};

static char* load_arg[] =
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

static char* launch_arg[] =
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

static char* launch_flag[] =
{
    "--none",
};

#endif