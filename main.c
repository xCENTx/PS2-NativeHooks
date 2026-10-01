#include <stdbool.h>
#include <assert.h>
#include "games/SOCOM/structs.h"
#include "games/SOCOM/game.h"
#include "ui.h"

// ------------------------------------------------------------
// statics
// ------------------------------------------------------------

#define BONE_INVALID (-1)
static const s32 BoneChains[][6] =
{
    // Left arm -> neck
    {
        FT_BONE_lhand,
        FT_BONE_lforearm,
        FT_BONE_lbicep,
        FT_BONE_neck,
        BONE_INVALID
    },

    // Right arm -> neck
    {
        FT_BONE_rhand,
        FT_BONE_rforearm,
        FT_BONE_rbicep,
        FT_BONE_neck,
        BONE_INVALID
    },

    // Left leg -> lower spine
    {
        FT_BONE_ltoe,
        FT_BONE_lfoot,
        FT_BONE_lcalf,
        FT_BONE_lthigh,
        FT_BONE_spinelo,
        BONE_INVALID
    },

    // Right leg -> lower spine
    {
        FT_BONE_rtoe,
        FT_BONE_rfoot,
        FT_BONE_rcalf,
        FT_BONE_rthigh,
        FT_BONE_spinelo,
        BONE_INVALID
    },

    // Spine -> head
    {
        FT_BONE_spinelo,
        FT_BONE_spinehi,
        FT_BONE_neck,
        FT_BONE_head,
        BONE_INVALID
    }
};
#define BONE_CHAIN_COUNT (sizeof(BoneChains) / sizeof(BoneChains[0]))

static const s32 BoundingBoxEdges[12][2] =
{
    {0,1}, {1,2}, {2,3}, {3,0},
    {4,5}, {5,6}, {6,7}, {7,4},
    {0,4}, {1,5}, {2,6}, {3,7}
};

// Each step is 5.625degrees:
// - every entry for 64 vertices
// - every 2nd for 32 vertices
// - every 4th for 16 vertices
// - every 8th for 8 vertices
static const f32 circle_cos[CIRCLE_SEGMENTS + 1] =
{
     1.000000f,
     0.995185f,
     0.980785f,
     0.956940f,
     0.923880f,
     0.881921f,
     0.831470f,
     0.773010f,
     0.707107f,
     0.634393f,
     0.555570f,
     0.471397f,
     0.382683f,
     0.290285f,
     0.195090f,
     0.098017f,
     0.000000f,
    -0.098017f,
    -0.195090f,
    -0.290285f,
    -0.382683f,
    -0.471397f,
    -0.555570f,
    -0.634393f,
    -0.707107f,
    -0.773010f,
    -0.831470f,
    -0.881921f,
    -0.923880f,
    -0.956940f,
    -0.980785f,
    -0.995185f,
    -1.000000f,
    -0.995185f,
    -0.980785f,
    -0.956940f,
    -0.923880f,
    -0.881921f,
    -0.831470f,
    -0.773010f,
    -0.707107f,
    -0.634393f,
    -0.555570f,
    -0.471397f,
    -0.382683f,
    -0.290285f,
    -0.195090f,
    -0.098017f,
     0.000000f,
     0.098017f,
     0.195090f,
     0.290285f,
     0.382683f,
     0.471397f,
     0.555570f,
     0.634393f,
     0.707107f,
     0.773010f,
     0.831470f,
     0.881921f,
     0.923880f,
     0.956940f,
     0.980785f,
     0.995185f,
     1.000000f
};

static const f32 circle_sin[CIRCLE_SEGMENTS + 1] =
{
     0.000000f,
     0.098017f,
     0.195090f,
     0.290285f,
     0.382683f,
     0.471397f,
     0.555570f,
     0.634393f,
     0.707107f,
     0.773010f,
     0.831470f,
     0.881921f,
     0.923880f,
     0.956940f,
     0.980785f,
     0.995185f,
     1.000000f,
     0.995185f,
     0.980785f,
     0.956940f,
     0.923880f,
     0.881921f,
     0.831470f,
     0.773010f,
     0.707107f,
     0.634393f,
     0.555570f,
     0.471397f,
     0.382683f,
     0.290285f,
     0.195090f,
     0.098017f,
     0.000000f,
    -0.098017f,
    -0.195090f,
    -0.290285f,
    -0.382683f,
    -0.471397f,
    -0.555570f,
    -0.634393f,
    -0.707107f,
    -0.773010f,
    -0.831470f,
    -0.881921f,
    -0.923880f,
    -0.956940f,
    -0.980785f,
    -0.995185f,
    -1.000000f,
    -0.995185f,
    -0.980785f,
    -0.956940f,
    -0.923880f,
    -0.881921f,
    -0.831470f,
    -0.773010f,
    -0.707107f,
    -0.634393f,
    -0.555570f,
    -0.471397f,
    -0.382683f,
    -0.290285f,
    -0.195090f,
    -0.098017f,
     0.000000f
};

#define AIM_FOV 100.0f
#define AIM_FOV_SQ (AIM_FOV * AIM_FOV)

// ------------------------------------------------------------
// Helper Methods
// ------------------------------------------------------------

// ------------------------------------------------------------
// Tiny math / value helpers
// ------------------------------------------------------------

static inline Vec3 Vec3_Add(Vec3 a, Vec3 b)
{
    Vec3 out =
    {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    };

    return out;
}

static inline Vec3 QuaternionRotate(Vec4 q, Vec3 v)
{
    // q.xyz = imaginary component
    // q.w   = real component

    Vec3 qv =
    {
        q.x,
        q.y,
        q.z
    };

    Vec3 uv =
    {
        qv.y * v.z - qv.z * v.y,
        qv.z * v.x - qv.x * v.z,
        qv.x * v.y - qv.y * v.x
    };

    Vec3 uuv =
    {
        qv.y * uv.z - qv.z * uv.y,
        qv.z * uv.x - qv.x * uv.z,
        qv.x * uv.y - qv.y * uv.x
    };

    float s = 2.0f * q.w;

    Vec3 out =
    {
        v.x + (uv.x * s) + (uuv.x * 2.0f),
        v.y + (uv.y * s) + (uuv.y * 2.0f),
        v.z + (uv.z * s) + (uuv.z * 2.0f)
    };

    return out;
}

static inline Vec3 TransformPoint(const Matrix4x4* m, Vec3 p)
{
    Vec3 out;

    // Matrix convention based on your translation being m[3][0..2]
    out.x =
        p.x * m->m[0][0] +
        p.y * m->m[1][0] +
        p.z * m->m[2][0] +
              m->m[3][0];

    out.y =
        p.x * m->m[0][1] +
        p.y * m->m[1][1] +
        p.z * m->m[2][1] +
              m->m[3][1];

    out.z =
        p.x * m->m[0][2] +
        p.y * m->m[1][2] +
        p.z * m->m[2][2] +
              m->m[3][2];

    return out;
}

static inline Vec4 TransformPoint4(const Matrix4x4* m, Vec4 v)
{
    Vec4 out;
    out.x =
        v.x * m->m[0][0] +
        v.y * m->m[1][0] +
        v.z * m->m[2][0] +
        v.w * m->m[3][0];

    out.y =
        v.x * m->m[0][1] +
        v.y * m->m[1][1] +
        v.z * m->m[2][1] +
        v.w * m->m[3][1];

    out.z =
        v.x * m->m[0][2] +
        v.y * m->m[1][2] +
        v.z * m->m[2][2] +
        v.w * m->m[3][2];

    out.w =
        v.x * m->m[0][3] +
        v.y * m->m[1][3] +
        v.z * m->m[2][3] +
        v.w * m->m[3][3];

    return out;
}

static inline f32 FastAbs(f32 x)
{
    return x < 0.0f ? -x : x;
}

static inline f32 FastLength2D(f32 x, f32 y)
{
    f32 ax;
    f32 ay;
    f32 maxv;
    f32 minv;

    ax = FastAbs(x);
    ay = FastAbs(y);

    if (ax > ay)
    {
        maxv = ax;
        minv = ay;
    }
    else
    {
        maxv = ay;
        minv = ax;
    }

    return maxv + (minv * 0.375f);
}

static inline GSXYZ2 MakeGSVertex(f32 x, f32 y)
{
    GSXYZ2 v;

    v.x = (s32)(x * 16.0f);
    v.y = (s32)(y * 16.0f);

    v.z = 1000000000;
    v.w = 0;

    v.x -= 0x9400;
    v.y -= 0x8E00;

    return v;
}

static inline void SetGSColor( GSRGBAQ *out, Vec4 color)
{
    f32 rgba[4];

    rgba[0] = color.x * 255.0f;
    rgba[1] = color.y * 255.0f;
    rgba[2] = color.z * 255.0f;
    rgba[3] = color.w * 128.0f;

    sceVu0FTOI0Vector( (s32 *)out, rgba );
}

static void MakeBoundingBox(CNode* node, Vec3 world[8])
{
    Vec3 min = node->m_bounds.m_min;
    Vec3 max = node->m_bounds.m_max;

    world[0] = (Vec3){ min.x, min.y, min.z };
    world[1] = (Vec3){ max.x, min.y, min.z };
    world[2] = (Vec3){ max.x, max.y, min.z };
    world[3] = (Vec3){ min.x, max.y, min.z };

    world[4] = (Vec3){ min.x, min.y, max.z };
    world[5] = (Vec3){ max.x, min.y, max.z };
    world[6] = (Vec3){ max.x, max.y, max.z };
    world[7] = (Vec3){ min.x, max.y, max.z };

    for (int i = 0; i < 8; i++)
        world[i] = TransformPoint(&node->m_mtx, world[i]);
}

// ------------------------------------------------------------
// Bigger math / camera helpers
// ------------------------------------------------------------

static Matrix4x4 MatrixMultiply(Matrix4x4 a, Matrix4x4 b)
{
    Matrix4x4 out;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            out.m[i][j] = 0.0f;
            for (int k = 0; k < 4; k++)
            {
                out.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    return out;
}

static bool WorldToScreen(Vec3 world, Vec2* screen)
{
	if (!screen)
		return false;
	
	Matrix4x4 mtxWorldToScreen;
	u32 pWorld = *(u32*)gWorld;
	if (!pWorld)
		return false;

	u32 pCamera = *(u32*)(pWorld + 0xC0);
	if (!pCamera)
		return false;

	u32 pScratch = *(u32*)(pCamera + 0x3C0);
	if (!pScratch)
		return false;

	// obtain camera
	CZCamera* camera = (CZCamera*)pCamera;

	sceVu0CopyMatrix((s64)&mtxWorldToScreen, (s64)(pScratch + 0x20)); // 0000000070003E80 <--- correct matrix being passed

	Vec4 input = { world.x, world.y, world.z, 1.0f };
	Vec4 gs = TransformPoint4(&mtxWorldToScreen, input);

	// behind camera check
	if (gs.w <= 0.001f)
		return false;

    f32 invW = 1.f / gs.w;

	f32 projectedX = gs.x * invW;
	f32 projectedY = gs.y * invW;

	//	get screen dimensions
	s32 width = camera->m_screen.right - camera->m_screen.left; // 640 width
	s32 height = camera->m_screen.bottom - camera->m_screen.top; // 448 height
	if (width <= 0 || height <= 0)
		return false;

	// native pixel coordinates
	f32 x = projectedX - camera->m_screen.left;
	f32 y = projectedY - camera->m_screen.top;
    if (x < 0 || y < 0 || x > width || y > height)
		return false;

	screen->x = x;
	screen->y = y;

    return true;
}


// ------------------------------------------------------------
// Container Helpers
// ------------------------------------------------------------
typedef void (*ZArrayForEach_Callback)(void* obj, void* ctx);
typedef void* (*ZArrayFind_Callback)(void* obj, void* ctx);

// iterates on each entity
static void ZArray_ForEach(ZArray* arr, ZArrayForEach_Callback cb, void* ctx)
{
    ZIterator* it;
    ZIterator* end;

    if (arr == 0 || cb == 0 || arr->count == 0 || arr->begin == 0 || arr->end == 0)
        return;

    it = (ZIterator*)arr->begin;

    if (it == 0)
        return;

    end = (ZIterator*)it->prev;

    if (end == 0)
        return;

    do
    {
        if (it->data != 0)
            cb((void*)it->data, ctx);

        it = (ZIterator*)it->next;

    } while (it != 0 && it->data != end->data);
}

// returns the first object in the array matching the input
static void* ZArray_Find(ZArray* arr, ZArrayFind_Callback cb, void* ctx)
{
    ZIterator* it;
    ZIterator* end;

    if (arr == 0 || cb == 0 || arr->count == 0 || arr->begin == 0 || arr->end == 0)
        return 0;

    it = (ZIterator*)arr->begin;

    if (it == 0)
        return 0;

    end = (ZIterator*)it->prev;

    if (end == 0)
        return 0;

    do
    {
        if (it->data != 0)
        {
            void* result = cb((void*)it->data, ctx);

            if (result != 0)
                return result;
        }

        it = (ZIterator*)it->next;

    } while (it != 0 && it->data != end->data);

    return 0;
}

// ------------------------------------------------------------
// Entity Helpers
// ------------------------------------------------------------

static Vec3 GetBoneModelPosition(CZBodyPart* bone)
{
    Vec3 position = bone->mOrigin;

    CZBodyPart* parent = bone->pParentBone;

    // Safety limit because there are only 33 skeleton entries.
    // Prevents an invalid/cyclic parent pointer from hanging the game.
    int depth = 0;

    while (parent != 0 && depth < 33)
    {
        position = QuaternionRotate(
            parent->mRotation,
            position
        );

        position = Vec3_Add(
            position,
            parent->mOrigin
        );

        parent = parent->pParentBone;

        depth++;
    }

    return position;
}

static Vec3 GetBoneWorldPosition(CZSealBody* seal, CZBodyPart* bone)
{
    Vec3 modelPosition = GetBoneModelPosition(bone);

    return TransformPoint(
        &seal->m_ent.p_Node->m_mtx,
        modelPosition
    );
}

static bool GetBoneWorldPosByIndex(CZSealBody* seal, FT_BONE idx, Vec3* wsOrigin)
{
    if (seal == 0 || wsOrigin == 0)
        return false;

    CZBodyPart* bone = seal->m_Skeleton[idx];
    if (!bone)
        return false;

    *wsOrigin = GetBoneWorldPosition(seal, bone);

    return true;
}

static bool IsVisible(CZSealBody* fromSeal, CZSealBody* toSeal)
{
    if (fromSeal == 0 || toSeal == 0 || fromSeal->m_ent.m_TargetCount <= 0 || fromSeal->m_ent.p_TargetArray == 0)
        return false;

    for ( int i = 0; i < fromSeal->m_ent.m_TargetCount; i++)
    {
        CTarget* pTarget = &fromSeal->m_ent.p_TargetArray[i];

        if (pTarget->pEntity == toSeal)
            return pTarget->m_visible;
    }

    return false;
}

static bool GetMuzzleWorldLocation(CZSealBody* seal, Vec3* wsOrigin)
{
    if (!seal || !wsOrigin)
        return false;

    u32 tag = g_tagFirepoint_default;

    CZKit* kit = &seal->m_Kit;

    if (kit)
    {
        s32 index = kit->mCurrentWeaponIndex;
        if (index >= 0 && index < kit->mMaxWeaponIndex)
        {
            CZWeapon* weapon = kit->pWeapons[index];

            if ( weapon 
                && CZKit_IsLauncherWeapon(weapon) 
                && kit->mWeaponFireTypes[index] >= 4
            )
            {
                tag = g_tagFirepoint_203;
            }
        }
    }
    
    return CZSealBody_GetFirepointPos(
        seal,
        &wsOrigin->x,
        tag
    );
}

static bool IsAlive(CZSealBody* seal)
{
    return seal && (seal->m_ent.m_EntityBits & ENTITY_IS_ALIVE);
}

static bool IsCharacter(CEntity* entity)
{
    if (!entity)
        return false;

    CNode* node = entity->p_Node;
    
    return node 
        && (node->mBits & CNODE_CHARACTER) != 0 
        && entity->mEntityType == ENTITY_SEAL;
}

static bool IsAlphaUnit(CZSealBody* seal)
{
    u32 mask = seal->m_ent.m_TeamMask;

    return (mask & FIRETEAM_MASK(FT_FIRETEAM)) &&
           (mask & FIRETEAM_MASK(FT_ALPHA));
}

static bool IsBravoUnit(CZSealBody* seal)
{
    u32 mask = seal->m_ent.m_TeamMask;

    return (mask & FIRETEAM_MASK(FT_FIRETEAM)) &&
           (mask & FIRETEAM_MASK(FT_BRAVO));
}

static bool IsSealTeamUnit(CZSealBody* seal)
{
    u32 mask = seal->m_ent.m_TeamMask;

    return (mask & FIRETEAM_MASK(FT_FIRETEAM)) && (mask & (FIRETEAM_MASK(FT_ALPHA) | FIRETEAM_MASK(FT_BRAVO)));
}

static bool SealJoinFireteam(CZSealBody* seal, u32 team)
{
    if (!seal || !seal->m_ent.p_SealCtrl)
        return false;

    CSealUnit* unit = CSealUnit_GetUnitByTeam(team);

    if (!unit)
        return false;

    // Already attached to this CSealUnit.
    if (seal->m_ent.p_SealCtrl->p_unit == unit)
        return true;

    // Add the game's fireteam + requested team memberships.
    CEntity_JoinTeam(&seal->m_ent, FT_FIRETEAM);
    CEntity_JoinTeam(&seal->m_ent, team);

    // Move the character into the actual CSealUnit.
    CSealUnit_SealJoinUnit(unit, seal);

    return seal->m_ent.p_SealCtrl->p_unit == unit;
}

static bool RespawnSeal(CZSealBody* seal)
{
    if (!seal 
        || IsAlive(seal) == true 
        || IsCharacter(&seal->m_ent) == false
    )
    {
        return false;
    }

    // Request native respawn.
    seal->m_should_respawn = true;

    // Force CHUD through its menu-state cleanup transition.
    if (seal->m_ent.p_SealCtrl)
    {
        seal->m_ent.p_SealCtrl->m_menu_state = MENU_STATE_ORDERS;
        seal->m_ent.p_SealCtrl->m_menu_state = MENU_STATE_NONE;
    }
}

static bool RespawnLocalPlayer()
{
    CZSealBody* seal = ftsGetPlayer();
    CAppCamera* camera = (CAppCamera*)gAppCamera;
    if (!seal || !camera || IsAlive(seal))
        return false;

    if (RespawnSeal(seal) == false)
        return false;


    // Restore camera ownership/state.
    camera->pAttachedPlayer = seal;
    camera->mCamDeathState = 0;

    return true;
}

static CZSealBody* SpawnAIBot(CCharacterType* character, const char* name, u32 teamMask, Matrix4x4 position, u32* outId)
{
    if (!character || !name)
        return 0;

    CZSealBody* bot = ftsCreateSeal(character, name, CTRL_AI);
    if (!bot)
        return 0;

    /*
    
        zdb::CNode::SetName(*(int *)(Seal + 0x28), a2);
        if ( *(_BYTE *)(a2 + 0x50) )
            CEntity::SetDisplayName(v21, (int)a2 + 0x50);
        (*(void (__fastcall **)(__int64, _QWORD, _QWORD))(*(_DWORD *)v21 + 60))(v21, (int)&v47, (int)&v44); // teleport
        CEntity::SetTeamMask(v21, v13);
        v22 = 1LL;
        v23 = 1;
        if ( (*(_BYTE *)(a2 + 260) & 8) == 0 )
        v22 = 0LL;
        if ( !v22 )
        v23 = 0;
        *(_BYTE *)(v21 + 221) = *(_BYTE *)(v21 + 221) & 0xFE | v23 & 1;
        v24 = ((__int64 (*)(void))*(_DWORD *)(**(_DWORD **)(v21 + 192) + 56))();
        v25 = v24;
        if ( v24 )
        {
            v26 = 1LL;
            v27 = v24 + 1500;
            v28 = 1;
            v29 = 1LL;
            v30 = 1;
            *(_BYTE *)(v24 + 640) &= ~1u;
            if ( (*(_BYTE *)(a2 + 260) & 4) == 0 )
                v26 = 0LL;
            if ( !v26 )
                v28 = 0;
            *(_BYTE *)(v24 + 640) = *(_BYTE *)(v24 + 640) & 0xDF | (32 * (v28 & 1));// CSealCtrlAi::SetDebug
            if ( (*(_BYTE *)(a2 + 260) & 0x20) == 0 )
                v29 = 0LL;
            if ( !v29 )
                v30 = 0;
            *(_BYTE *)(v24 + 641) = *(_BYTE *)(v24 + 641) & 0xFB | (4 * (v30 & 1));
            AI_PARAMS::__as((int)v24 + 1500, (int)a2 + 200);
            *(_DWORD *)(v27 + 56) = a2;
            CSealCtrlAi::InitAiBrain(v25, v31, v32);
            result = v21;
        }
        else
        {
            if ( a3 )
            {
                if ( (*(_DWORD *)(a1 + 176) & 0x1000LL) != 0 )
                *(_BYTE *)(v21 + 3972) = *(_BYTE *)(v21 + 3972) & 0xF7 | 8;
                if ( (*(_DWORD *)(a1 + 176) & 0x2000LL) != 0 )
                *(_BYTE *)(v21 + 3972) = *(_BYTE *)(v21 + 3972) & 0xEF | 0x10;
            }
            result = v21;
        }
    
    */

    
    CNode_SetName(bot->m_ent.p_Node, name);
    
    CEntity_SetDisplayName(&bot->m_ent, name);

    CEntity_SetTeamMask(&bot->m_ent, teamMask);

    CZSealBody_TeleportTo(bot, &position);

    if (outId)
        *outId = bot->m_ent.m_id;

    return bot;
}

static CZSealBody* SpawnFriendlyAIBot(const char* name, u32* outId)
{
    if (!name || !outId)
        return 0;

    CZSealBody* local_seal = ftsGetPlayer();
    if (!local_seal)
        return 0;

    CCharacterType* character = CZSealBody_GetCharacter(local_seal);
    if (!character)
        return 0;

    Matrix4x4 tm = local_seal->m_ent.m_Matrix;
    tm.m[3][0] = local_seal->m_ReticlePt.x;
    tm.m[3][1] = local_seal->m_ReticlePt.y;
    tm.m[3][2] = local_seal->m_ReticlePt.z;

    CZSealBody* bot = SpawnAIBot(character, name, local_seal->m_ent.m_TeamMask, tm, outId);
    if (!bot)
        return 0;

    return bot;
}

static bool RemoveBot(u8 id)
{

}

// ------------------------------------------------------------
// Message System
// ------------------------------------------------------------

static void Notification(const char* msg, float timer) { C2DMessage_AddMessage((void*)0x4D4990, msg, timer); }

// ------------------------------------------------------------
// Draw Primitives
// ------------------------------------------------------------

// invokes Draw2DLine
static void spDrawLineGradient(float x1, float y1, float x2, float y2, Vec4 color_start, Vec4 color_end)
{
    float start[4] =
    {
        x1,
        y1,
        0.0f,
        1.0f
    };

    float end[4] =
    {
        x2,
        y2,
        0.0f,
        1.0f
    };
    
    float colorA[4] =
    {
        color_start.x,
        color_start.y,
        color_start.z,
        color_start.w
    };

    float colorB[4] =
    {
        color_end.x,
        color_end.y,
        color_end.z,
        color_end.w
    };

    Draw2DLine(start, end, colorA, colorB);
}

static void spDrawLine(float x1, float y1, float x2, float y2, Vec4 color)
{
    spDrawLineGradient(x1, y1, x2, y2, color, color);
}

static void spDrawLineRGB(float x1, float y1, float x2, float y2, Vec3 color)
{
    spDrawLine(x1, y1, x2, y2, (Vec4){ color.x, color.y, color.z, 1.0f });
}

// invokes RenderLineWorld : if (clip3DLine == true) Draw3DLine
static void wsDrawLineGradient(Vec3 a, Vec3 b, Vec4 colorA, Vec4 colorB)
{
    float start[4] =
    {
        a.x,
        a.y,
        a.z,
        1.0f
    };

    float end[4] =
    {
        b.x,
        b.y,
        b.z,
        1.0f
    };
    
    float color_start[4] =
    {
        colorA.x,
        colorA.y,
        colorA.z,
        colorA.w
    };

    float color_end[4] =
    {
        colorB.x,
        colorB.y,
        colorB.z,
        colorB.w
    };

    RenderLineWorld(start, end, color_start, color_end);
}

static void wsDrawLine(Vec3 start, Vec3 end, Vec4 color)
{
    wsDrawLineGradient(start, end, color, color);
}

static void wsDrawLineRGB(Vec3 start, Vec3 end, Vec3 color)
{
    wsDrawLine(start, end, (Vec4){ color.x, color.y, color.z, 1.0f} );
}

static void DrawStringTest(C2DString* string, C2DFont* font, void* camera, s32 x, s32 y)
{
	C2DString_Load(string, "HELLO WORLD", font, x, y);
	C2DString_Draw(string, camera);
}

// rewrite of Draw2DLine with AA
static void Draw2DLineNative(f32 x1, f32 y1, f32 x2, f32 y2, Vec4 color_start, Vec4 color_end)
{
    GSLinePacket *packet;
    u64 prim;

    packet = (GSLinePacket *)zSysSprGetPacket_FPP1(0);

    if (!packet)
        return;

    /*
     * GS PRIM
     *
     * 0x01 = LINE
     * 0x08 = IIP
     * 0x40 = ABE
     * 0x80 = AA1
     */
    prim = 0x89; /* LINE | IIP | AA1 */

    if (color_start.w != 1.0f || color_end.w != 1.0f)
        prim |= 0x40;

    /*
     * GIF tag
     *
     * NLOOP = 2
     * PRE   = 1
     * PRIM  = prim
     * NREG  = 2
     */
    packet->gif_tag = (prim << 47) | 0x2000400000008002ULL;

    /*
     * Packed register descriptor:
     *
     * RGBAQ
     * XYZ2
     */
    packet->gif_regs = 0x41;

    /*
     * Vertex 0
     */
    SetGSColor( &packet->vertices[0].rgba, color_start );

    packet->vertices[0].xyz = MakeGSVertex(x1, y1);

    /*
     * Vertex 1
     */
    SetGSColor( &packet->vertices[1].rgba, color_end );

    packet->vertices[1].xyz = MakeGSVertex(x2, y2);

    /*
     * DMA tag
     *
     * SOCOM original:
     *
     * +00 = 0x10000005
     * +04 = 0
     * +08 = 0x11000000
     * +0C = 0x50000005
     */
    packet->dma[0] = 0x10000005;
    packet->dma[1] = 0x00000000;
    packet->dma[2] = 0x11000000;
    packet->dma[3] = 0x50000005;

    /*
     * Total packet size = 6 qwords
     */
    zSysFifoKick( packet, 6 );
}

// rewrite of Draw3DLine with AA
static void Draw3DLineNative(Vec4 start, Vec4 end, Vec4 color_start, Vec4 color_end)
{
    GSLinePacket *packet;
    u32 pWorld;
    u32 pCamera;
    u32 pScratch;
    f32 *matrix;
    u64 prim;

    packet = (GSLinePacket *)zSysSprGetPacket_FPP1(0);
    if (!packet)
        return;

    /*
     * Match the game's matrix lookup:
     *
     * zdb__CWorld__m_world
     *      -> +0xC0
     *      -> +0x3C0
     *      -> +0x20
     */
	pWorld = *(u32*)gWorld;
    if (!pWorld)
        return;

	pCamera = *(u32*)(pWorld + 0xC0);
	if (!pCamera)
		return;

	pScratch = *(u32*)(pCamera + 0x3C0);
	if (!pScratch)
		return;

    matrix = (f32 *)(pScratch + 0x20);

    /*
     * Original game:
     *
     * 0x09 = LINE | IIP
     *
     * Our version:
     *
     * 0x89 = LINE | IIP | AA1
     */
    prim = 0x89;

    /*
     * The original enables ABE if either endpoint
     * color has alpha != 1.0f.
     */
    if (color_start.w != 1.0f || color_end.w   != 1.0f)
    {
        prim |= 0x40;
    }

    /*
     * GIF tag
     *
     * NLOOP = 2
     * EOP   = 1
     * PRE   = 1
     * PRIM  = prim
     * NREG  = 2
     *
     * Register list:
     * RGBAQ
     * XYZ2
     */
    packet->gif_tag = (prim << 47) | 0x2000400000008002ULL;

    packet->gif_regs = 0x41;

    /*
     * --------------------------------------------------
     * START VERTEX
     * --------------------------------------------------
     */

    SetGSColor( &packet->vertices[0].rgba, color_start );

    /*
     * Original Draw3DLine forces source W = 1.0f.
     */
    start.w = 1.0f;

    sceVu0RotTransPers( (s32 *)&packet->vertices[0].xyz, matrix, (f32 *)&start, 0 );

    /*
     * Original:
     *
     * lw   a2, 8(s4)
     * lw   v1, dword_48CEF8+0x58
     * addu v0, a2, v1
     * sw   v0, 8(s4)
     */
    packet->vertices[0].xyz.z += *(s32*)g_GS_Z_OFFSET;

    /*
     * --------------------------------------------------
     * END VERTEX
     * --------------------------------------------------
     */

    SetGSColor( &packet->vertices[1].rgba, color_end );

    end.w = 1.0f;

    sceVu0RotTransPers( (s32 *)&packet->vertices[1].xyz, matrix, (f32 *)&end, 0 );

    packet->vertices[1].xyz.z += *(s32 *)g_GS_Z_OFFSET;

    /*
     * DMA tag
     *
     * Packet is 6 QW total.
     * DMA payload = 5 QW.
     */
    packet->dma[0] = 0x10000005;
    packet->dma[1] = 0x00000000;
    packet->dma[2] = 0x11000000;
    packet->dma[3] = 0x50000005;

    zSysFifoKick(packet, 6);
}

static void RenderLineWorldNative(Vec4 start, Vec4 end, Vec4 color_start, Vec4 color_end)
{
    u32 pWorld;
    u32 pCamera;

	pWorld = *(u32*)gWorld;
    if (!pWorld)
        return;

	pCamera = *(u32*)(pWorld + 0xC0);
	if (!pCamera)
		return;

    //  if (clipLine3D((s64)(pCamera + 0x3D0), &start, &end))
    //      return;

    Draw3DLineNative(start, end, color_start, color_end);
}

// custom draw circle method
static void Draw2DCircle( f32 center_x, f32 center_y, f32 radius, f32 thickness, Vec4 color )
{
    GSCirclePacket *packet;
    f32 inner_radius;
    f32 outer_radius;
    f32 cx;
    f32 sy;
    f32 outer_x;
    f32 outer_y;
    f32 inner_x;
    f32 inner_y;
    u64 prim;
    s32 i;
    s32 v;
    s32 total_qw;
    s32 dma_qwc;

    if (radius <= 0.0f)
        return;

    if (thickness <= 0.0f)
        return;

    inner_radius = radius - (thickness * 0.5f);
    outer_radius = radius + (thickness * 0.5f);

    if (inner_radius < 0.0f)
        inner_radius = 0.0f;

    packet = (GSCirclePacket *)zSysSprGetPacket_FPP1(0);

    if (!packet)
        return;

    /*
     * TRIANGLE_STRIP | IIP
     */
    prim = 0x0C;

    if (color.w != 1.0f)
        prim |= 0x40;

    /*
     * NLOOP = number of vertices
     * PRE   = 1
     * NREG  = 2
     *
     * register list:
     * RGBAQ
     * XYZ2
     */
    packet->gif_tag = (prim << 47) | 0x2000400000008000ULL | (u64)CIRCLE_VERTICES;

    packet->gif_regs = 0x41;

    /*
     * Build the ring as:
     *
     * outer0
     * inner0
     * outer1
     * inner1
     * ...
     * outer16
     * inner16
     */
    v = 0;

    for (i = 0; i <= CIRCLE_SEGMENTS; i++)
    {
        cx = circle_cos[i];
        sy = circle_sin[i];

        outer_x = center_x + (cx * outer_radius);
        outer_y = center_y + (sy * outer_radius);

        inner_x = center_x + (cx * inner_radius);
        inner_y = center_y + (sy * inner_radius);

        /*
         * Outer vertex
         */
        SetGSColor( &packet->vertices[v].rgba, color );

        packet->vertices[v].xyz = MakeGSVertex( outer_x, outer_y );

        v++;

        /*
         * Inner vertex
         */
        SetGSColor( &packet->vertices[v].rgba, color );

        packet->vertices[v].xyz = MakeGSVertex( inner_x, inner_y );

        v++;
    }

    /*
     * Packet size:
     *
     * DMA       = 1 QW
     * GIF       = 1 QW
     * vertices  = 34 * 2 QW
     *
     * total = 70 QW
     */
    total_qw = 2 + (CIRCLE_VERTICES * 2);

    /*
     * DMA payload excludes DMA tag itself.
     */
    dma_qwc = total_qw - 1;

    packet->dma[0] = 0x10000000 | dma_qwc;
    packet->dma[1] = 0x00000000;
    packet->dma[2] = 0x11000000;
    packet->dma[3] = 0x50000000 | dma_qwc;

    zSysFifoKick( packet, total_qw );
}

// ------------------------------------------------------------
// Draw Primitives - Smoothing
// ------------------------------------------------------------

static void DrawSmooth2DLineNative( f32 x1, f32 y1, f32 x2, f32 y2, f32 width, Vec4 color_start, Vec4 color_end)
{
    GSLineStripPacket *packet;
    f32 dx;
    f32 dy;
    f32 length;
    f32 nx;
    f32 ny;
    f32 half_width;
    u64 prim;

    Vec2 p0;
    Vec2 p1;
    Vec2 p2;
    Vec2 p3;

    packet = (GSLineStripPacket *)zSysSprGetPacket_FPP1(0);

    if (!packet)
        return;

    dx = x2 - x1;
    dy = y2 - y1;

    length = FastLength2D(dx, dy);

    if (length <= 0.0001f)
        return;

    /*
     * Perpendicular direction.
     */
    nx = -dy / length;
    ny =  dx / length;

    half_width = width * 0.5f;

    nx *= half_width;
    ny *= half_width;

    /*
     * Triangle strip:
     *
     * p0 -------- p2
     * |           |
     * |           |
     * p1 -------- p3
     */
    p0.x = x1 + nx;
    p0.y = y1 + ny;

    p1.x = x1 - nx;
    p1.y = y1 - ny;

    p2.x = x2 + nx;
    p2.y = y2 + ny;

    p3.x = x2 - nx;
    p3.y = y2 - ny;

    /*
     * 0x04 = TRIANGLE_STRIP
     * 0x08 = IIP
     * 0x80 = AA1
     */
    prim = 0x0C;

    if (color_start.w != 1.0f || color_end.w != 1.0f)
        prim |= 0x40; /* ABE */

    /*
     * NLOOP = 4
     * PRE   = 1
     * NREG  = 2
     *
     * Registers = RGBAQ, XYZ2
     */
    packet->gif_tag = (prim << 47) | 0x2000400000008004ULL;

    packet->gif_regs = 0x41;

    /*
     * Start edge.
     */
    SetGSColor( &packet->vertices[0].rgba, color_start );

    packet->vertices[0].xyz = MakeGSVertex(p0.x, p0.y);

    SetGSColor( &packet->vertices[1].rgba, color_start );

    packet->vertices[1].xyz = MakeGSVertex(p1.x, p1.y);

    /*
     * End edge.
     */
    SetGSColor( &packet->vertices[2].rgba, color_end );

    packet->vertices[2].xyz = MakeGSVertex(p2.x, p2.y);

    SetGSColor( &packet->vertices[3].rgba, color_end );

    packet->vertices[3].xyz = MakeGSVertex(p3.x, p3.y);

    /*
     * Packet size:
     *
     * DMA      = 1 QW
     * GIF tag  = 1 QW
     * vertices = 8 QW
     *
     * total = 10 QW
     *
     * DMA QWC = 9
     */
    packet->dma[0] = 0x10000009;
    packet->dma[1] = 0x00000000;
    packet->dma[2] = 0x11000000;
    packet->dma[3] = 0x50000009;

    zSysFifoKick(packet, 10);
}

static void DrawFeathered2DLine( f32 x1, f32 y1, f32 x2, f32 y2, f32 width, f32 feather, Vec4 color_start, Vec4 color_end )
{
    GSFeatherLinePacket *packet;

    f32 dx;
    f32 dy;
    f32 length;
    f32 inv_length;

    f32 nx;
    f32 ny;

    f32 inner_half;
    f32 outer_half;

    f32 inner_nx;
    f32 inner_ny;

    f32 outer_nx;
    f32 outer_ny;

    Vec4 start_outer;
    Vec4 end_outer;

    u64 prim;

    /*
     * Direction.
     */
    dx = x2 - x1;
    dy = y2 - y1;

    length = FastLength2D(dx, dy);

    if (length <= 0.0001f)
        return;

    inv_length = 1.0f / length;

    /*
     * Unit perpendicular.
     */
    nx = -dy * inv_length;
    ny =  dx * inv_length;

    /*
     * width = solid center width
     * feather = fade distance on EACH side
     *
     * Example:
     *
     * width   = 1.5
     * feather = 1.0
     *
     * actual overall width = 3.5
     */
    inner_half = width * 0.5f;
    outer_half = inner_half + feather;

    inner_nx = nx * inner_half;
    inner_ny = ny * inner_half;

    outer_nx = nx * outer_half;
    outer_ny = ny * outer_half;

    /*
     * Outer colors retain RGB but fade completely transparent.
     */
    start_outer = color_start;
    end_outer   = color_end;

    start_outer.w = 0.0f;
    end_outer.w   = 0.0f;

    packet = (GSFeatherLinePacket *)zSysSprGetPacket_FPP1(0);

    if (!packet)
        return;

    /*
     * TRIANGLE_STRIP | IIP | ABE
     *
     * 0x04 = TRIANGLE_STRIP
     * 0x08 = IIP / Gouraud shading
     * 0x40 = ABE / alpha blending
     *
     * ABE is always required here because the outer rails
     * deliberately use alpha 0.
     */
    prim = 0x4C;

    /*
     * GIF:
     *
     * NLOOP = 8
     * PRE   = 1
     * NREG  = 2
     *
     * registers:
     * RGBAQ
     * XYZ2
     */
    packet->gif_tag = (prim << 47) | 0x2000400000008008ULL;

    packet->gif_regs = 0x41;

    /*
     * --------------------------------------------------
     * OUTER LEFT
     * --------------------------------------------------
     */

    SetGSColor( &packet->vertices[0].rgba, start_outer );

    packet->vertices[0].xyz = MakeGSVertex( x1 + outer_nx, y1 + outer_ny );

    SetGSColor( &packet->vertices[1].rgba, end_outer );

    packet->vertices[1].xyz = MakeGSVertex( x2 + outer_nx, y2 + outer_ny );

    /*
     * --------------------------------------------------
     * INNER LEFT
     * --------------------------------------------------
     */

    SetGSColor( &packet->vertices[2].rgba, color_start );

    packet->vertices[2].xyz = MakeGSVertex( x1 + inner_nx, y1 + inner_ny );

    SetGSColor( &packet->vertices[3].rgba, color_end );

    packet->vertices[3].xyz = MakeGSVertex( x2 + inner_nx, y2 + inner_ny );

    /*
     * --------------------------------------------------
     * INNER RIGHT
     * --------------------------------------------------
     */

    SetGSColor( &packet->vertices[4].rgba, color_start );

    packet->vertices[4].xyz = MakeGSVertex( x1 - inner_nx, y1 - inner_ny );

    SetGSColor( &packet->vertices[5].rgba, color_end );

    packet->vertices[5].xyz = MakeGSVertex( x2 - inner_nx, y2 - inner_ny );

    /*
     * --------------------------------------------------
     * OUTER RIGHT
     * --------------------------------------------------
     */

    SetGSColor( &packet->vertices[6].rgba, start_outer );

    packet->vertices[6].xyz = MakeGSVertex( x1 - outer_nx, y1 - outer_ny );

    SetGSColor( &packet->vertices[7].rgba, end_outer );

    packet->vertices[7].xyz = MakeGSVertex( x2 - outer_nx, y2 - outer_ny );

    /*
     * 18 QW packet
     * 17 QW following the DMA tag.
     */
    packet->dma[0] = 0x10000011;
    packet->dma[1] = 0x00000000;
    packet->dma[2] = 0x11000000;
    packet->dma[3] = 0x50000011;

    zSysFifoKick(packet, 18);
}

// ------------------------------------------------------------
// Draw Helpers
// ------------------------------------------------------------

// draws a bounding box around the input object in world space
static void wsDrawBoundingBox(CNode* node, Vec3 color)
{
    Vec3 world[8];

    if (node == 0)
        return;

    MakeBoundingBox(node, world);    

    for (int i = 0; i < 12; i++)
    {
        int a = BoundingBoxEdges[i][0];
        int b = BoundingBoxEdges[i][1];

        wsDrawLineRGB(world[a], world[b], color);
    }
}

// draws a bounding box around the input object in screen space
static void spDrawBoundingBox(CNode* node, Vec3 color)
{
    Vec3 world[8];
    Vec2 screen[8];
    bool visible[8];

    Vec4 drawColor =
    {
        color.x,
        color.y,
        color.z,
        1.0f
    };

    if (node == 0)
        return;

    MakeBoundingBox(node, world);    

    // project each corner independently
    for (int i = 0; i < 8; i++)
        visible[i] = WorldToScreen( world[i], &screen[i] );

    // draw
    for (int i = 0; i < 12; i++)
    {
        int a = BoundingBoxEdges[i][0];
        int b = BoundingBoxEdges[i][1];
        
        if (visible[a] && visible[b])
            DrawSmooth2DLineNative(screen[a].x, screen[a].y, screen[b].x, screen[b].y, 0.5f, drawColor, drawColor);
    }
}

// connects all skeleton points on a czseal and draws in both world and canvas spaces
static void wsDrawSkeleton(CZSealBody* seal)
{
    if (seal == 0 || seal->m_ent.p_Node == 0)
        return;

    for (int limb = 0; limb < BONE_CHAIN_COUNT; limb++)
    {
        Vec3 previousPosition;
        int havePrevious = 0;

        for (int i = 0; i < 6; i++)
        {
            int boneIndex = BoneChains[limb][i];

            if (boneIndex == BONE_INVALID)
                break;

            CZBodyPart* bone = seal->m_Skeleton[boneIndex];

            if (bone == 0)
            {
                havePrevious = 0;
                continue;
            }

            Vec3 currentPosition = GetBoneWorldPosition(seal, bone);

            if (!havePrevious)
            {
                previousPosition = currentPosition;
                havePrevious = 1;
                continue;
            }


		    // Draw the 3D line as well
            wsDrawLineRGB( previousPosition, currentPosition, (Vec3){ 1.0f, 1.0f, 1.0f } );
            

            previousPosition = currentPosition;
        }
    }
}

// draws a player skeleton using custom bone indexing - renders in screen space
static void spDrawSkeleton(CZSealBody* seal)
{
    Vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

    if (seal == 0 || seal->m_ent.p_Node == 0)
        return;

    for (int limb = 0; limb < BONE_CHAIN_COUNT; limb++)
    {
        Vec2 previousScreen;
        bool previousVisible = false;
        bool havePrevious = false;

        for (int i = 0; i < 6; i++)
        {
            int boneIndex = BoneChains[limb][i];

            if (boneIndex == BONE_INVALID)
                break;

            CZBodyPart* bone = seal->m_Skeleton[boneIndex];

            if (bone == 0)
            {
                havePrevious = false;
                continue;
            }

            Vec3 world = GetBoneWorldPosition(seal, bone);
            Vec2 screen;
            bool visible = WorldToScreen(world, &screen);
            if (havePrevious && previousVisible && visible)
            {
                DrawSmooth2DLineNative(
                    previousScreen.x,
                    previousScreen.y,
                    screen.x,
                    screen.y,
                    0.5f,
                    color,
                    color
                );
            }
            previousScreen = screen;
            previousVisible = visible;
            havePrevious = true;
        }
    }
}


// ------------------------------------------------------------
// Patcher
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

static inline void Patch_FlushCache(void)
{
    FlushCache(0); // D-cache writeback
    FlushCache(2); // I-cache invalidate
}

static inline u8 Patch_ReadU8(u32 address)
{
    return *(volatile u8*)address;
}

static inline u16 Patch_ReadU16(u32 address)
{
    return *(volatile u16*)address;
}

static inline u32 Patch_ReadU32(u32 address)
{
    return *(volatile u32*)address;
}

static inline f32 Patch_ReadFloat(u32 address)
{
    return *(volatile f32*)address;
}

static inline void Patch_U8(u32 address, u8 value)
{
    *(volatile u8*)address = value;
}

static inline void Patch_U16(u32 address, u16 value)
{
    *(volatile u16*)address = value;
}

static inline void Patch_U32(u32 address, u32 value)
{
    *(volatile u32*)address = value;
}

static inline void Patch_Float(u32 address, f32 value)
{
    *(volatile f32*)address = value;
}

static inline void Patch_Instruction(u32 address, u32 instruction)
{
    Patch_U32(address, instruction);

    Patch_FlushCache();
}

static inline bool Patch_InstructionChecked(u32 address, u32 expected, u32 replacement)
{
    if (Patch_ReadU32(address) != expected)
        return false;

    Patch_U32(address, replacement);

    Patch_FlushCache();

    return true;
}

static inline void Patch_Instructions(u32 address, const u32* instructions, u32 count)
{
    volatile u32* dst = (volatile u32*)address;

    for (u32 i = 0; i < count; i++)
        dst[i] = instructions[i];

    Patch_FlushCache();
}

static inline void Patch_NOP(u32 address)
{
    Patch_Instruction(address, 0x00000000u);
}

static inline void Patch_J(u32 address, u32 target)
{
    u32 instruction = 0x08000000u | ((target >> 2) & 0x03FFFFFFu);
    Patch_Instruction(address, instruction);
}

static inline void Patch_JAL(u32 address, u32 target)
{
    u32 instruction = 0x0C000000u | ((target >> 2) & 0x03FFFFFFu);
    Patch_Instruction(address, instruction);
}

bool Patch_ForceCompleteMission_enable(void)
{
    return Patch_InstructionChecked(
        fn_MissionTick_BEQ_MISSION_SUCCESS, 
        AUTO_COMPLETE_ORIGINAL,        
        AUTO_COMPLETE_PATCHED
    );
}

bool Patch_ForceCompleteMission_disable(void)
{
    return Patch_InstructionChecked(
        fn_MissionTick_BEQ_MISSION_SUCCESS, 
        AUTO_COMPLETE_PATCHED, 
        AUTO_COMPLETE_ORIGINAL
    );
}


bool Patch_ForceStart_enable(void)
{
    return Patch_InstructionChecked(
        fn_ToggleReady_JR_RA_FORCE_START,
        FORCE_START_ORIGINAL,
        FORCE_START_PATCHED
    );
}

bool Patch_ForceStart_disable(void)
{
    return Patch_InstructionChecked(
        fn_ToggleReady_JR_RA_FORCE_START,
        FORCE_START_PATCHED,
        FORCE_START_ORIGINAL
    );
}


bool Patch_NeverEnd_enable(void)
{
    return Patch_InstructionChecked(
        fn_MissionTick_JAL_MP_ROUND_END,
        NEVER_END_ORIGINAL,
        NEVER_END_PATCHED
    );
}

bool Patch_NeverEnd_disable(void)
{
    return Patch_InstructionChecked(
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
    CheatStateUpdate(
        &m_featureset[CHEAT_MATCH_FORCE_START],
        Patch_ForceStart_enable,
        Patch_ForceStart_disable
    );

    // Never Ending Match Patch
    CheatStateUpdate(
        &m_featureset[CHEAT_MATCH_NEVER_ENDS],
        Patch_NeverEnd_enable,
        Patch_NeverEnd_disable
    );
}

// ------------------------------------------------------------
// Menu
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
        || CNode_Rendered(pNode) == 0
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

static void ProcessPlayerFeatures(CZSealBody* seal)
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


// ------------------------------------------------------------
// Native Hook
// ------------------------------------------------------------
// CCameraApp::Tick -> CZSealBody::CheckDIShoot
__attribute__((section(".hook"), noinline))
void hk_CheckDIShoot(CZSealBody* seal, s64 a2, int a3)
{
    MenuEnsureInitialized();
    CZSealBody_CheckDIShoot(seal, a2, a3);
    Patches_Tick(); // process patches

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

    ProcessPlayerFeatures(seal);

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

//
__attribute__((section(".hook_teleport"), noinline))
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

    CZKit_HandleFireWeapon(kit, a2, a3, a4);
}
// Keep the original single-source build. Header guards prevent repeated game definitions.
#include "ui.c"
