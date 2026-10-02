#include "game.h"
#include "hook.h"
#include "ui.h"
#include "render.h"
#include "containers.h"
#include "entities.h"

// ------------------------------------------------------------
// statics
// ------------------------------------------------------------

#define NATIVE_THREAD_STACK_SIZE 0x4000

__attribute__((section(".thread_stack"), aligned(16)))
static u8 g_NativeThreadStack[NATIVE_THREAD_STACK_SIZE];
static volatile s32 g_NativeThreadId = -1;

#define AIM_FOV 100.0f
#define AIM_FOV_SQ (AIM_FOV * AIM_FOV)

// ------------------------------------------------------------
// Message System
// ------------------------------------------------------------

static void Notification(const char* msg, float timer) { C2DMessage_AddMessage((void*)0x4D4990, msg, timer); }


// ------------------------------------------------------------
// PATCHES
// ------------------------------------------------------------
typedef s8 CHEAT_FEATURES;
enum
{
    CHEAT_ESP,
    CHEAT_INFINITE_AMMO, 
    CHEAT_NO_RELOAD,
    CHEAT_PERFECT_SHOT,
    CHEAT_AIMBOT,
    CHEAT_TELEPORT_TO_XHAIR,
    CHEAT_MAGIC_BULLET,
    CHEAT_FPS,
    CHEAT_FOG,
    CHEAT_AI_FRIENDLY,
    CHEAT_AI_FFA,
    CHEAT_AI_RESPAWNS,
    CHEAT_AI_VISION,                
    CHEAT_MATCH_RESPAWN_PLAYER,         // 
    CHEAT_MATCH_FORCE_START,            // 
    CHEAT_MATCH_NEVER_ENDS,             // 
    CHEAT_MATCH_RESPAWN_MODE,           // 
    CHEAT_MATCH_AUTO_COMPLETE,        //    
    CHEAT_MATCH_SPAWN_BOT,
    CHEAT_MAX
};

typedef struct
{
    bool enabled;
    bool applied;
} CheatState;

typedef bool (*PatchFn_t)(void);

__attribute__((section(".cheats")))
static CheatState m_featureset[CHEAT_MAX] = { 0 };

static inline bool CheatStateChanged(const CheatState* cheat)
{ return cheat->enabled != cheat->applied; }

static inline void CheatStateApplied(CheatState* cheat)
{ cheat->applied = cheat->enabled; }

static inline void CheatStateUpdate(CheatState* cheat, PatchFn_t enable, PatchFn_t disable)
{
    if (CheatStateChanged(cheat) == false)
        return;

    bool success;
    if (cheat->enabled)
        success = enable();
    else
        success = disable();

    if (success)
        CheatStateApplied(cheat);
}

bool Patch_ForceCompleteMission_enable(void)
{
    return Memory_PatchInstructionChecked(
        fn_MissionTick_BEQ_MISSION_SUCCESS, 
        AUTO_COMPLETE_ORIGINAL,        
        AUTO_COMPLETE_PATCHED
    );
}

bool Patch_ForceCompleteMission_disable(void)
{
    return Memory_PatchInstructionChecked(
        fn_MissionTick_BEQ_MISSION_SUCCESS, 
        AUTO_COMPLETE_PATCHED, 
        AUTO_COMPLETE_ORIGINAL
    );
}

bool Patch_ForceStart_enable(void)
{
    return Memory_PatchInstructionChecked(
        fn_ToggleReady_JR_RA_FORCE_START,
        FORCE_START_ORIGINAL,
        FORCE_START_PATCHED
    );
}

bool Patch_ForceStart_disable(void)
{
    return Memory_PatchInstructionChecked(
        fn_ToggleReady_JR_RA_FORCE_START,
        FORCE_START_PATCHED,
        FORCE_START_ORIGINAL
    );
}

bool Patch_NeverEnd_enable(void)
{
    return Memory_PatchInstructionChecked(
        fn_MissionTick_JAL_MP_ROUND_END,
        NEVER_END_ORIGINAL,
        NEVER_END_PATCHED
    );
}

bool Patch_NeverEnd_disable(void)
{
    return Memory_PatchInstructionChecked(
        fn_MissionTick_JAL_MP_ROUND_END,
        NEVER_END_PATCHED,
        NEVER_END_ORIGINAL
    );
}

void Patches_Tick(void)
{
    // Auto Complete Missions Patch
    CZSealBody* seal = ftsGetPlayer();

    if (!seal)
    { 
        // disable so we dont get stuck in endless mission complete
         m_featureset[CHEAT_MATCH_AUTO_COMPLETE].enabled = false;
    }

    CheatStateUpdate(
        &m_featureset[CHEAT_MATCH_AUTO_COMPLETE],
        Patch_ForceCompleteMission_enable,
        Patch_ForceCompleteMission_disable
    );
    
    // Force Start Match Patch
    //  CheatStateUpdate(
    //      &m_featureset[CHEAT_MATCH_FORCE_START],
    //      Patch_ForceStart_enable,
    //      Patch_ForceStart_disable
    //  );

    // Never Ending Match Patch
    CheatStateUpdate(
        &m_featureset[CHEAT_MATCH_NEVER_ENDS],
        Patch_NeverEnd_enable,
        Patch_NeverEnd_disable
    );
}


// ------------------------------------------------------------
// MENU
// ------------------------------------------------------------

static const f32 menu_scales[] = {0.65f, 0.8f, 1.0f};
static const char* const menu_sizes[] = {"SMALL", "MEDIUM", "LARGE"};
static const char* const menu_colors[] = {"CYAN", "AMBER", "GREEN"};
static const char* const menu_pages[] = {"FEATURES", "NET MATCH", "MISSION", "SETTINGS"};
static const Vec4 menu_accents[] = {{65, 210, 235, 128}, {255, 195, 65, 128}, {24, 180, 40, 128}};
void MenuBuild(u32 pass, u32 pressed)
{
    UIContext ui;
    const UILayout layout = {28, 28, 584, 10};
    UIStyle style = {
        28,
        2,
        4,
        0.67f,
        24,
        8,
        4,
        {12, 18, 28, 112},
        {35, 58, 73, 112},
        {235, 240, 245, 128},
        {160, 174, 190, 128},
        {0, 0, 0, 0}
    };
    u32 page = g_menu.page;
    f32 scale = menu_scales[g_menu.scale_index];
    style.accent = menu_accents[g_menu.accent_index];
    UI_BeginWindow(&ui, &g_menu.ui[page], &layout, &style, pass, pressed);
    UI_Text(&ui, "SOCOM - NATIVE MENU", 1.0f, style.text);
    UI_Text(&ui, "SCUS 971.34", 0.65f, style.muted);
    UI_Spacing(&ui, 10);
    UI_Combo(&ui, 100, "PAGE", &g_menu.page, menu_pages, MENU_PAGE_COUNT, scale);

    switch ( page )
    {
        case MENU_PAGE_FEATURES:
        {
            UI_Separator(&ui, 1.0f, 6.0f, style.muted);
            UI_Checkbox(&ui, 1, "ESP", &m_featureset[CHEAT_ESP].enabled, scale);
            UI_Checkbox(&ui, 2, "INFINITE AMMO", &m_featureset[CHEAT_INFINITE_AMMO].enabled, scale);
            UI_Checkbox(&ui, 3, "NO RELOAD", &m_featureset[CHEAT_NO_RELOAD].enabled, scale);
            UI_Checkbox(&ui, 4, "PERFECT SHOT", &m_featureset[CHEAT_PERFECT_SHOT].enabled, scale);
            UI_Checkbox(&ui, 5, "AIMBOT", &m_featureset[CHEAT_AIMBOT].enabled, scale);
            UI_Checkbox(&ui, 6, "TELEPORT TO CROSSHAIR", &m_featureset[CHEAT_TELEPORT_TO_XHAIR].enabled, scale);
            break;
        }

        case MENU_PAGE_NET_MATCH:
        {
            UI_SeparatorText(&ui, "LOBBY", 0.65f, style.muted);

            if (UI_Checkbox(&ui, 201, "[H] FORCE START MATCH", &m_featureset[CHEAT_MATCH_FORCE_START].enabled, scale));

            UI_SeparatorText(&ui, "IN GAME", 0.65f, style.muted);

            UI_Checkbox(&ui, 202, "[H] MATCH NEVER ENDS", &m_featureset[CHEAT_MATCH_NEVER_ENDS].enabled, scale);

            UI_Checkbox(&ui, 203, "[H] RESPAWN MATCH", &m_featureset[CHEAT_MATCH_RESPAWN_MODE].enabled, scale);

            break;
        }

        case MENU_PAGE_MISSION:
        {
            UI_Separator(&ui, 1.0f, 6.0f, style.muted);

            // 
            if (UI_Button(&ui, 301, "RESPAWN LOCAL PLAYER", scale)) 
            { 
                m_featureset[CHEAT_MATCH_RESPAWN_PLAYER].enabled  = true;
            }

            if (UI_Button(&ui, 302, "SPAWN BOT", scale))
            {
                
                m_featureset[CHEAT_MATCH_SPAWN_BOT].enabled  = true;
            }

            // 
            UI_Checkbox(&ui, 303, "AUTO COMPLETE MISSION", &m_featureset[CHEAT_MATCH_AUTO_COMPLETE].enabled, scale);

            // ai entities assigned a unique team
            UI_Checkbox(&ui, 304, "FREE FOR ALL", &m_featureset[CHEAT_AI_FFA].enabled, scale);          

            // assigns all ai to a seal unit for control
            UI_Checkbox(&ui, 305, "AI FRIENDLY", &m_featureset[CHEAT_AI_FRIENDLY].enabled, scale);      
            
            // ai respawns on death
            UI_Checkbox(&ui, 306, "AI RESPAWNS", &m_featureset[CHEAT_AI_RESPAWNS].enabled, scale);      

            // render ai pathing and  ( draws a cone on the ground from feet to vision distance , line for pathing and circle around for hearing distance ? )   
            UI_Checkbox(&ui, 307, "AI VISION", &m_featureset[CHEAT_AI_VISION].enabled, scale);  

            break;
        }

        case MENU_PAGE_SETTINGS:
        {
            UI_Separator(&ui, 1.0f, 6.0f, style.muted);
            UI_Checkbox(&ui, 401, "WATERMARK", &g_menu.watermark, scale);
            UI_Combo(&ui, 402, "TEXT SIZE", &g_menu.scale_index, menu_sizes, 3, scale);
            UI_Combo(&ui, 403, "ACCENT COLOR", &g_menu.accent_index, menu_colors, 3, scale);
            if (UI_Button(&ui, 404, "RESET DISPLAY", scale))
            {
                g_menu.watermark = true;
                g_menu.scale_index = 1;
                g_menu.accent_index = 1;
            }

            break;
        }
    }

    UI_Spacing(&ui, 8);
    UI_Separator(&ui, 1.0f, 6.0f, style.muted);
    UI_Text(&ui, "D-PAD: MOVE / CHANGE    CROSS: SELECT", 0.6f, style.muted);
    UI_Spacing(&ui, 6);
    UI_Text(&ui, "L3 + R3: TOGGLE    CIRCLE: CLOSE", 0.6f, style.muted);
    UI_EndWindow(&ui);
}

void MenuDraw(void)
{
    if (g_menu.watermark)
    {
        const char* text = "PS2-NativeHooks by NightFyre";
        const f32 center_x = 320.0f;
        const f32 position_y = 424.0f;
        const f32 scale = 0.6f;

        DrawTextCentered(text, center_x + 1.0f, position_y + 1.0f, scale, (Vec4){0, 0, 0, 128});
        DrawTextCentered(text, center_x, position_y, scale, menu_accents[g_menu.accent_index]);
    }
    if (g_menu.open)
    {
        MenuBuild(UI_DRAW, 0);
    }
}


// ------------------------------------------------------------
// Feature Callbacks
// ------------------------------------------------------------
#define MAX_FRAME_PLAYERS 32
#define MAX_FRAME_PICKUPS 64

typedef struct
{
    CZSealBody* seal;
    CNode* node;

    bool isRendered;
    bool isAlive;
    bool isCharacter;
    bool isLocalFriendly; // whether the player is friendly to the local player seal unit
} FramePlayer;

typedef struct
{
    CPickup* pickup;
    CNode* node;
} FramePickup;

typedef struct
{
    CZSealBody* local;

    FramePlayer players[MAX_FRAME_PLAYERS];
    s32 playerCount;

    FramePickup pickups[MAX_FRAME_PICKUPS];
    s32 pickupCount;

    CZSealBody* aimTarget;
    f32 bestTargetDistSq;

    u32 frame;
} GameFrameContext;
static GameFrameContext g_Frame;

static void CollectPlayer(void* obj, void* context)
{
    CZSealBody* entity = (CZSealBody*)obj;
    GameFrameContext* ctx = (GameFrameContext*)context;
    
    if (!entity || !ctx || !ctx->local)
        return;

    if (entity == ctx->local)
        return;

    if (ctx->playerCount >= MAX_FRAME_PLAYERS)
        return;

    CNode* node = (CNode*)entity->m_ent.p_Node;
    if (!node)
        return;

    FramePlayer* player = &ctx->players[ctx->playerCount++];

    player->seal                = entity;
    player->node                = node;
    player->isCharacter         = IsCharacter(&entity->m_ent);
    player->isAlive             = IsAlive(entity);
    player->isLocalFriendly     = IsSealTeamUnit(entity);
    player->isRendered          = CNode_Rendered(node);

}

static void CollectPickup(void* obj, void* context)
{
    CPickup* pickup = (CPickup*)obj;
    GameFrameContext* ctx = (GameFrameContext*)context;

    if (!pickup || !ctx)
        return;

    if (ctx->pickupCount >= MAX_FRAME_PICKUPS)
        return;

    CNode* node = (CNode*)pickup->pNode;
    if (!node)
        return;

    FramePickup* framePickup = &ctx->pickups[ctx->pickupCount++];
    framePickup->pickup = pickup;
    framePickup->node = node;
}

static void GameFrame_Collect(CZSealBody* local)
{
    g_Frame.local = local;

    g_Frame.playerCount = 0;
    g_Frame.pickupCount = 0;
    g_Frame.aimTarget = NULL;
    g_Frame.bestTargetDistSq = 99999999.0f;
    g_Frame.frame++;
    
    ZArray_ForEach((ZArray*)gSealArray, CollectPlayer, &g_Frame);
    ZArray_ForEach((ZArray*)gPickupArray, CollectPickup, &g_Frame);
}


typedef struct
{
    CZSealBody* seal;
    CZSealBody* target;
    f32 bestTargetDistSq;
} ctxPlayerESP;
static void ProcessPlayers(void* obj, void* ctx)
{
    CZSealBody* entity;
    ctxPlayerESP* esp;
    CNode* pNode;
    bool isSealTeam;
    bool isAlive;
    bool isCharacter;
    bool isRendered;
    f32 dx;
    f32 dy;
    f32 aimDistSq;

    entity = (CZSealBody*)obj;
    esp = (ctxPlayerESP*)ctx;

    if (entity == 0 || esp == 0 || esp->seal == 0)
        return;

    if (entity == esp->seal)
        return;

    pNode = (CNode*)entity->m_ent.p_Node;

    if (pNode == 0)
        return;

    isAlive = IsAlive(entity);
    isCharacter = IsCharacter(&entity->m_ent);
    isSealTeam = IsSealTeamUnit(entity);
    isRendered = CNode_Rendered(pNode);

    //  
    if ((m_featureset[CHEAT_MATCH_RESPAWN_MODE].enabled  || m_featureset[CHEAT_AI_RESPAWNS].enabled  ) 
        && isCharacter 
        && !isAlive
    )
    {
        // need to ensure is host for a FORCE ALL RESPAWN
        RespawnSeal(entity);
    }

    //
    if (m_featureset[CHEAT_AI_FRIENDLY].enabled  
        && isAlive 
        && isCharacter
        && !isSealTeam
    )
    {
        SealJoinFireteam(entity, FT_BRAVO);
    }

    if ( isSealTeam 
        || isCharacter == false
        || isAlive == false
        || isRendered == false
    )
        return;
    
    if (m_featureset[CHEAT_ESP].enabled  )
    {
        //  wsDrawBoundingBox(pNode);
        spDrawSkeleton(entity);
    }
    
    if (!m_featureset[CHEAT_AIMBOT].enabled  )
        return;

    Vec2 screen;
    Vec3 wsBoneHead;
    if (IsVisible(esp->seal, entity) == false || GetBoneWorldPosByIndex(entity, FT_BONE_head, &wsBoneHead) == false || WorldToScreen(wsBoneHead, &screen) == false)
        return;

    dx = screen.x - 320.0f;
    dy = screen.y - 224.0f;
    aimDistSq = dx * dx + dy * dy;
    if (aimDistSq >= AIM_FOV_SQ || aimDistSq >= esp->bestTargetDistSq)
        return;
    
    esp->bestTargetDistSq = aimDistSq;
    esp->target = entity;
}

typedef struct
{
    CZSealBody* seal;
    CPickup* target;
    f32 bestTargetDistSq;
} ctxPickupESP;
static void ProcessPickupESP(void* obj, void* ctx)
{
    CPickup* pickup;
    ctxPickupESP* esp;
    CNode* pNode;
    f32 dx;
    f32 dy;
    f32 aimDistSq;
    
    pickup = (CPickup*)obj;
    esp = (ctxPickupESP*)ctx;

    if (pickup == 0 || esp == 0)
        return;
    
    pNode = (CNode*)pickup->pNode;
    if (pNode == 0)
        return;
    
    wsDrawBoundingBox(pNode, (Vec3){ 0.5f, 0.5f, 0.0f });

    Vec2 screen;
    if (WorldToScreen((Vec3){ pNode->m_mtx.m[3][0], pNode->m_mtx.m[3][1], pNode->m_mtx.m[3][2] }, &screen) == false)
        return;

    dx = screen.x - 320.0f;
    dy = screen.y - 224.0f; 
    aimDistSq = dx * dx + dy * dy;  
    if (aimDistSq >= AIM_FOV_SQ || aimDistSq >= esp->bestTargetDistSq)
        return;

    esp->bestTargetDistSq = aimDistSq;
    esp->target = pickup;
}

static void PlayerSeal_Tick(CZSealBody* seal)
{
    if (!seal)
        return;
    
    CZKit* kit = &seal->m_Kit;
    
    // infinite ammo
    if (m_featureset[CHEAT_INFINITE_AMMO].enabled  )
    {
        for (int i = 0; i < sizeof(kit->pWeapons) / sizeof(kit->pWeapons[0]); i++)
        {
            CZWeapon* pWeapon = kit->pWeapons[i];
            if (!pWeapon)
                continue;

            s32 newAmmo =  pWeapon->szMags;
            
            switch (i)
            {
                case 0:
                {
                    s32 capacity = (s32)(sizeof(kit->mPrimaryMags) / sizeof(kit->mPrimaryMags[0]));
                    for (int j = 0; j < pWeapon->defaultMags && j < capacity; j++)
                        kit->mPrimaryMags[j] = newAmmo;
                    break;
                }
                
                case 1:
                {
                    s32 capacity = (s32)(sizeof(kit->mSecondaryMags) / sizeof(kit->mSecondaryMags[0]));
                    for (int j = 0; j < pWeapon->defaultMags && j < capacity; j++)
                        kit->mSecondaryMags[j] = newAmmo;
                    break;
                }
                
                case 2: kit->mEqSlot1Ammo = newAmmo; break;
                case 3: kit->mEqSlot2Ammo = newAmmo; break;
                case 4: kit->mEqSlot3Ammo = newAmmo; break;
            }
        }
    }
    
    // perfect shot
    if (m_featureset[CHEAT_PERFECT_SHOT].enabled  )
    {
        seal->m_ShoulderRecoil = 0.0f;

        kit->mRecoilPunch = (Vec2){ 0.0f, 0.0f }; // remove recoil punch 
        kit->mPrevRecoilPunch = (Vec2){ 0.0f, 0.0f }; // remove recoil offset
        kit->mRifleKick = (Vec3){ 0.0f, 0.0f, 0.0f }; // remove kick offset
        kit->mScreenOffset = (Vec2){ 0.0f, 0.0f }; // maintains scope 0
        kit->mHeartbeatState = 0; // prevents the heartbeat state from simulating
        kit->mHeartbeat = (Vec2){ 0.0f, 0.0f }; // removes heartbeat vibrations and sound
        kit->mFireRifleKickState = 0; // prevents the kick state from incrementing
        kit->mWeaponFireCount = 0; // allows full auto fire in scope , wont get kicked to fps view. if set to 0 it allows full auto fire for all weapons
        // missing crosshair bloom
    }

    // no reload time / rechamber
    if (m_featureset[CHEAT_NO_RELOAD].enabled  )
    {
        for (int i = 0; i < sizeof(kit->pWeapons) / sizeof(kit->pWeapons[0]); i++)
        {
            CZWeapon* pWeapon = kit->pWeapons[i];
            if (!pWeapon)
                continue;

            pWeapon->bReloadAfterShot = false; // permanent setting
            pWeapon->mReloadTime = 0.0f; // permanent setting
        }
    }

    if (m_featureset[CHEAT_MATCH_RESPAWN_PLAYER].enabled  )
    {
        m_featureset[CHEAT_MATCH_RESPAWN_PLAYER].enabled  = false;
        RespawnLocalPlayer();
    }

}

static void PlayerESP_Tick(CZSealBody* seal, ctxPlayerESP* ctx)
{

}

static void PickupESP_Tick(CPickup* pickup, ctxPickupESP* ctx)
{

}




// ------------------------------------------------------------
// HOOKS
// ------------------------------------------------------------

DefineHookFor(CZSealBody_Tick);
DefineHookFor(CZSealBody_CheckDIShoot);
DefineHookFor(CZKit_HandleFireWeapon);
DefineHookFor(zVid_ZTestOn);
DefineHookFor(recoTick);
DefineHookFor(CHUD_PauseGame);
DefineHookFor(CMission_OnMissionComplete);
DefineHookFor(CZPersonaState_ToggleReady);
DefineHookFor(CAppCamera_Tick);

__attribute__((noinline))
void hk_HandleFireWeapon(CZKit* kit, s64 a2, s64 a3, float a4)
{

    MenuEnsureInitialized();

    Vec3 firepoint;
    CZSealBody* local_seal = ftsGetPlayer();
    CZSealBody* this_seal = (CZSealBody*)kit->pSealBody;
    bool validAim = local_seal && (local_seal->m_AimWorldPos.x != 0.0f || local_seal->m_AimWorldPos.y != 0.0f || local_seal->m_AimWorldPos.z != 0.0f);

    if (m_featureset[CHEAT_TELEPORT_TO_XHAIR].enabled  
        && local_seal && local_seal == this_seal 
        && validAim
        && GetMuzzleWorldLocation(local_seal, &firepoint)
    )
    {
        Matrix4x4 teleport = local_seal->m_ent.m_Matrix;
        
        //  target location
        teleport.m[3][0] = local_seal->m_ReticlePt.x;
        teleport.m[3][1] = local_seal->m_ReticlePt.y;
        teleport.m[3][2] = local_seal->m_ReticlePt.z;

        // Teleport
        CZSealBody_TeleportTo(local_seal, &teleport);

        Notification("Teleporting to location", 0.0f);

        return;
    }

    CZKit_HandleFireWeapon_Original(kit, a2, a3, a4);
}

__attribute__((noinline))
void hk_ToggleReady(void)
{
    CZPersonaState_ToggleReady_Original();

    if (m_featureset[CHEAT_MATCH_FORCE_START].enabled)
        UIForceMPLaunch();
}

__attribute__((noinline))
void hk_OnMissionComplete(CMission* pMission, MISSION_STATE dwState)
{
    bool bSafe = dwState == MISSION_UNLOADED || dwState == MISSION_ABORTED || dwState == MISSION_TIMEOUT;

    if (bSafe || m_featureset[CHEAT_MATCH_AUTO_COMPLETE].enabled) 
    {
        CMission_OnMissionComplete_Original(pMission, dwState);
        return;
    }
    
    if ( m_featureset[CHEAT_MATCH_NEVER_ENDS].enabled)
    {
        return;
    }

    CMission_OnMissionComplete_Original(pMission, dwState);
}

__attribute__((noinline))
void hk_CheckDIShoot(CZSealBody* seal, s64 a2, int a3)
{

    MenuEnsureInitialized();
    
    CZSealBody_CheckDIShoot_Original(seal, a2, a3);

    if (seal == 0 || seal != ftsGetPlayer())
        return;

    ctxPlayerESP ctxPlayers = { 0 };
    ctxPlayers.seal = seal;
    ctxPlayers.bestTargetDistSq = 99999999.0f;
    ZArray_ForEach((ZArray*)gSealArray, ProcessPlayers, &ctxPlayers);

    if (m_featureset[CHEAT_ESP].enabled  )
    {
        ctxPickupESP ctxPickups = { 0 };
        ctxPickups.seal = seal;
        ctxPickups.bestTargetDistSq = 99999999.0f;

        ZArray_ForEach((ZArray*)gPickupArray, ProcessPickupESP, &ctxPickups);
    }

    //  PlayerSeal_Tick(seal);

    if (m_featureset[CHEAT_MATCH_SPAWN_BOT].enabled)
    {
        m_featureset[CHEAT_MATCH_SPAWN_BOT].enabled = false;

        u32 id;
        if (SpawnFriendlyAIBot("NightFyre", &id))
            Notification("Spawned BOT", 0.0f);
    }

    // aimbot
    if (m_featureset[CHEAT_AIMBOT].enabled  )
    {   
        if (ctxPlayers.target)
        {
            Vec2 screen[2];
            Vec3 targetOrigin[2];
            if (GetBoneWorldPosByIndex(ctxPlayers.target, FT_BONE_head, &targetOrigin[0]) 
                && WorldToScreen(targetOrigin[0], &screen[0])
                && CZSealBody_GetFirepointPos(seal, &targetOrigin[1].x, 0x46B500) && WorldToScreen(targetOrigin[1], &screen[1]))
            {
                seal->m_ReticlePt = targetOrigin[0];    
                seal->m_AimPoint = targetOrigin[0];    
                //  float start[4] = {320.f, 224.f, 0.f, 1.f};
                //  float end[4] = {screen.x, screen.y, 0.0f, 1.0f};
                //  float color_start[4] = {1.0f, 1.0f, 1.0f, 0.3f};
                //  float color_end[4] = {1.f, 0.0f, 0.0f, 0.75f};
                //  Draw2DLine(start, end, color_start, color_end);
                //  DrawSmooth2DLineNative(screen[0].x, screen[0].y, screen[1].x, screen[1].y, 0.5f, (Vec4){1.0f, 1.0f, 1.0f, 0.5f}, (Vec4){1.0f, 0.0f, 0.0f, 0.5f} );

                // draw laser ( beautiful )
                float start[4] = {targetOrigin[1].x, targetOrigin[1].y, targetOrigin[1].z, 1.f};
                float end[4] = {targetOrigin[0].x, targetOrigin[0].y, targetOrigin[0].z, 1.f};
                float color_start[4] = {1.0f, 1.0f, 1.0f, 0.5f};
                float color_end[4] = {1.0f, 0.0f, 0.0f, 0.5f};
                RenderLineWorld(start, end, color_start, color_end);

                // Draw 
                // DrawFeathered2DLine( screen[0].x, screen[0].y, screen[1].x, screen[1].y, 0.5f, 0.75f, (Vec4){1.0f, 1.0f, 1.0f, 0.5f}, (Vec4){1.0f, 0.0f, 0.0f, 0.5f} );

            }
        }

        // draw aim fov
        Draw2DCircle(320.f, 224.f, AIM_FOV, 1.0f, (Vec4){1.0f, 1.0f, 1.0f, 1.0f });
    }


    // [RENDER] teleport to crosshair destination / valid
    Vec3 firepoint;
    bool validAim = seal->m_AimWorldPos.x != 0.0f || seal->m_AimWorldPos.y != 0.0f || seal->m_AimWorldPos.z != 0.0f;
    if ( m_featureset[CHEAT_TELEPORT_TO_XHAIR].enabled  
        && validAim
        && CZSealBody_GetFirepointPos(seal, &firepoint.x, 0x46B500)
    )
    {
        float start[4] = {firepoint.x, firepoint.y, firepoint.z, 1.f};
        float end[4] = {seal->m_ReticlePt.x, seal->m_ReticlePt.y, seal->m_ReticlePt.z, 1.f};
        float color_start[4] = {1.0f, 1.0f, 1.0f, 0.5f};
        float color_end[4] = {0.0f, 1.0f, 0.0f, 0.5f};
        RenderLineWorld(start, end, color_start, color_end);
    }
}

__attribute__((noinline))
void hk_CZSealBody_Tick(CZSealBody* seal, float deltaTime)
{
    CZSealBody_Tick_Original(seal, deltaTime);
}

__attribute__((noinline))
void hk_CAppCamera_Tick(CAppCamera* camera)
{
    CAppCamera_Tick_Original(camera);
}


// ------------------------------------------------------------
// NAITIVE HOOKS 
// ------------------------------------------------------------
static bool g_bNativeHooksBootstrapped  = false;
static volatile bool g_bNativeHooksRunning = false;
static volatile u32 g_NativeHooksGP = 0;

static bool NativeHooks_Init(void)
{
    if (!CreateDetourChecked(CZKit_HandleFireWeapon, hk_HandleFireWeapon))
        return false;

    if (!CreateDetourChecked(CZPersonaState_ToggleReady, hk_ToggleReady))
        return false;

    if (!CreateDetourChecked(CMission_OnMissionComplete, hk_OnMissionComplete))
        return false;

    if (!CreateDetourChecked(CZSealBody_CheckDIShoot, hk_CheckDIShoot))
        return false;

    if (!CreateDetourChecked(CZSealBody_Tick, hk_CZSealBody_Tick))
        return false;

    if (!CreateDetourChecked(CAppCamera_Tick, hk_CAppCamera_Tick))
        return false;

    MenuEnsureInitialized();

    Memory_FlushCache();

    return true;
}

static void NativeHooks_Shutdown(void)
{
    g_bNativeHooksRunning = false;

    RemoveDetour(CZKit_HandleFireWeapon);
    RemoveDetour(CZPersonaState_ToggleReady);
    RemoveDetour(CMission_OnMissionComplete);
    RemoveDetour(CZSealBody_CheckDIShoot);
    RemoveDetour(CZSealBody_Tick);
    RemoveDetour(CAppCamera_Tick);

    Memory_FlushCache();
}

static void NativeHooks_Thread(void* arg)
{
    (void)arg;

    g_bNativeHooksRunning = NativeHooks_Init();

    //  while (g_bNativeHooksRunning)
    //  {
    //  
    //      PlayerSeal_Tick();
    //      
    //      Patches_Tick(); // process patches
    //   
    //      // mcDelayThread(1);    // H-SYNC
    //  }
    
    //  NativeHooks_Shutdown();

    g_NativeThreadId = -1;

    ExitDeleteThread();
}

static s32 NativeHooks_CreateThread(void)
{
    ee_thread_t thread = { 0 };

    __asm__ volatile("move %0, $gp" : "=r"(g_NativeHooksGP)); // capture gp register

    thread.func             = NativeHooks_Thread;
    thread.stack            = g_NativeThreadStack;
    thread.stack_size       = sizeof(g_NativeThreadStack);
    thread.gp_reg           = (void*)g_NativeHooksGP;
    thread.initial_priority = 0x40;

    s32 threadId = CreateThread(&thread);

    if (threadId < 0)
        return threadId;

    s32 result = StartThread(threadId, NULL);

    if (result < 0)
    {
        DeleteThread(threadId);
        return result;
    }

    g_NativeThreadId = threadId;

    return threadId;
}


#define BOOTSTRAP_CALL_ADDRESS  0x0020ED50

__attribute__((section(".bootstrap"), noinline))
s64 Bootstrap(CZNetwork* network)
{
    if (!g_bNativeHooksBootstrapped)
    {
        g_bNativeHooksBootstrapped  = true;

        Memory_SetFlushCacheFunction(FlushCache);

        NativeHooks_CreateThread();

        // Restore CGame::Tick -> CZNetwork::zNetUpdate.
        // Memory_MakeCall(BOOTSTRAP_CALL_ADDRESS, (u32)CZNetwork_zNetUpdate);
    }

    if (g_bNativeHooksRunning)
    {
        
        Patches_Tick(); // process patches
    }

    return CZNetwork_zNetUpdate(network);
}
