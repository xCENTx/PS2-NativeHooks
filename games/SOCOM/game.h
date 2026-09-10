#ifndef SOCOM_GAME_H
#define SOCOM_GAME_H

#include "structs.h"

#define SOCOM_ELF_PATH "cdrom0:\\SCUS_971.34;1"


// ------------------------------------------------------------
// Globals
// ------------------------------------------------------------
#define gHud 0x48E594
#define gWorld 0x48D848
#define gCamera 0x51E778
#define gSealArray 0x4D46A0
#define gAppCamera 0x48D488
#define tickAnimFireWeapon 0x1B7C90

// ------------------------------------------------------------
// Native functions
// ------------------------------------------------------------
typedef void(*sceVu0CopyMatrix_t)(s64 out, s64 in); // copies a matrix from 'in' to 'out'
#define sceVu0CopyMatrix ((sceVu0CopyMatrix_t)0x1518A8)

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