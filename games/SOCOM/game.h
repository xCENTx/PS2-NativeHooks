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

#define g_MessageQue 0x4D4990

#define g_vft_BatchRelocator 0x48A520
#define g_vft_2DString_2 0x48A730
#define g_CInput_pads 0x48C8D0

#define g_theNetwork_2 0x52A5A9
#define g_tagFirepoint_default 0x46B500
#define g_tagFirepoint_203 0x46B4F0
#define g_net_isHost 0x52A5A8
// ------------------------------------------------------------
// Native functions
// ------------------------------------------------------------

typedef void(*C2DString_Load_t)(C2DString* self, const char* text, C2DFont* font, int x, int y); // loads the string with the specified font and position
#define C2DString_Load ((C2DString_Load_t)0x319340)

typedef void(*C2DString_Draw_t)(C2DString* self, CZCamera* camera); // draws the string using the specified camera
#define C2DString_Draw ((C2DString_Draw_t)0x3196D0)

typedef void(*AI_LineTo_t)(float* a1); // draws a line to point connecting the previous point
#define AI_LineTo ((AI_LineTo_t)0x2F81A0)

typedef void(*AI_DrawLine_t)(u64 mask, float* start, float* end); // draws a line in 3d space
#define AI_DrawLine ((AI_DrawLine_t)0x2F8030)

typedef void(*RenderLineWorld_t)(float* start, float* end, float* color, float* color_2); // draws a line in world space
#define RenderLineWorld ((RenderLineWorld_t)0x249590)

typedef void(*Draw2DLine_t)(float* start, float* end, float* color, float* color_2); // draws a line on the canvas
#define Draw2DLine ((Draw2DLine_t)0x316FB0)

typedef void(*Draw3DLine_t)(float* start, float* end, float* color, float* color_2); // draws a line in world space
#define Draw3DLine ((Draw3DLine_t)0x317220)

typedef bool(*clip3DLine_t)(s64 a1, s64 a2, s64 a3); 
#define Clip3DLine ((clip3DLine_t)0x316C90)

typedef s64(*GetCharacterHit_t)(CZSealBody* a1, float* a2, s64 a3); // CZSealBody* seal , Vec3* outOrigin, ???
#define GetCharacterHit ((GetCharacterHit_t)0x2A8200)

typedef void(*CZSealBody_CheckDIShoot_t)(CZSealBody* a1, s64 a2, int a3); // CZSealBody* seal, Vec3 wsOrigin, Matrix4x4 mtx 
#define CheckDIShoot ((CZSealBody_CheckDIShoot_t)0x002A7490)

typedef bool(*CNode_Rendered_t)(CNode* pNode);  //  returns whether a node is set to be rendered or not
#define CNode_Rendered ((CNode_Rendered_t)0x22A4B0)

typedef CZSealBody*(*ftsGetPlayer_t)(); // gets the local player
#define ftsGetPlayer ((ftsGetPlayer_t)0x00200010)

typedef void(*ftsSetPlayer_t)(CZSealBody* pPlayer); // sets the local player
#define ftsSetPlayer ((ftsSetPlayer_t)0x00200020)

typedef u64(*zSysSprGetPacket__FPP1_t)(s32 a1);
#define zSysSprGetPacket_FPP1 ((zSysSprGetPacket__FPP1_t)0x30D1E0)

typedef void(*sceVu0CopyMatrix_t)(s64 out, s64 in); // copies a matrix from 'in' to 'out'
#define sceVu0CopyMatrix ((sceVu0CopyMatrix_t)0x1518A8)

typedef void(*sceVu0FTOI0Vector_t)(s32* a1, f32* a2);
#define sceVu0FTOI0Vector ((sceVu0FTOI0Vector_t)0x1518E0)

typedef void(*sceVu0CopyVector_t)(void* a1, void* a2);
#define sceVu0CopyVector ((sceVu0CopyVector_t)0x151898)

typedef void(*sceVu0RotTransPers_t)(s32* dst, f32* mtx, f32* src, s32 mode);
#define sceVu0RotTransPers ((sceVu0RotTransPers_t)0x1520E8)

typedef void(*zSysFifoKick_t)(void* a1, s32 a2);
#define zSysFifoKick ((zSysFifoKick_t)0x30CFB0)

typedef bool(*CZSealBody_GetFirepointPos_t)(CZSealBody* a1, float *a2, s64 a3);
#define CZSealBody_GetFirepointPos ((CZSealBody_GetFirepointPos_t)0x2D1720)

typedef bool(*CZKit_IsLauncherWeapon_t)(CZWeapon* a1);
#define CZKit_IsLauncherWeapon ((CZKit_IsLauncherWeapon_t)0x32EB90)

typedef bool(*CZKit_WeaponIsGrenadeLauncherMode_t)(CZKit *a1, s64 a2);
#define CZKit_WeaponIsGrenadeLauncherMode ((CZKit_WeaponIsGrenadeLauncherMode_t)0x2B6710)

// not used
typedef void(*CZKit_WillFireWeapon_t)(CZKit* a1, float a2);
#define CZKit_WillFireWeapon ((CZKit_WillFireWeapon_t)0x2B9140)

// hook for teleporting on fire event
typedef void(*CZKit_HandleFireWeapon_t)(CZKit* a1, s64 a2, s64 a3, float a4);
#define CZKit_HandleFireWeapon ((CZKit_HandleFireWeapon_t)0x2B7730)

// used for drawing messages
typedef s64(*C2DMessage_Q_Add_Message_t)(void* a1, const char* msg, float a3); // adds a message to the message que top center of screen
#define C2DMessage_AddMessage ((C2DMessage_Q_Add_Message_t)0x202C70)

typedef s64(*CZSealBody_TeleportTo_t)(CZSealBody* seal, const Matrix4x4* matrix);
#define CZSealBody_TeleportTo ((CZSealBody_TeleportTo_t)0x2537D0)

// hook for rendering menu
typedef void(*zVid_ZTestOn_t)();
#define zVid_ZTestOn ((zVid_ZTestOn_t)0x31ED90)

// hook for capturing input
typedef u64(*recoTick_t)();
#define recoTick ((recoTick_t)0x352B70)

// hook to pause the game
typedef u64(*CHUD_PauseGame_t)();
#define CHUD_PauseGame ((CHUD_PauseGame_t)0x3D9DF0)

typedef void(*C2DString_MakePacket_t)(void*, const u32* relocator, int depth);
#define C2DString_MakePacket ((C2DString_MakePacket_t)0x319750)

typedef void(*C2DPoly_MakePacket_t)(void*, void*);
#define C2DPoly_MakePacket ((C2DPoly_MakePacket_t)0x3180F0)



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