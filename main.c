#include "games/SOCOM/structs.h"
#include "games/SOCOM/game.h"

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


// ------------------------------------------------------------
// Helper Methods
// ------------------------------------------------------------

static inline __attribute__((always_inline))
Vec3 Vec3_Add(Vec3 a, Vec3 b)
{
    Vec3 out =
    {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    };

    return out;
}

static inline __attribute__((always_inline))
Vec3 QuaternionRotate(Vec4 q, Vec3 v)
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

static inline __attribute__((always_inline))
Matrix4x4 MatrixMultiply(Matrix4x4 a, Matrix4x4 b)
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

static inline __attribute__((always_inline))
Vec3 TransformPoint(const Matrix4x4* m, Vec3 p)
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

static inline __attribute__((always_inline))
Vec4 TransformPoint4(const Matrix4x4* m, Vec4 v)
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

static inline __attribute__((always_inline))
bool WorldToScreen(Vec3 world, Vec2* screen)
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

static inline __attribute__((always_inline))
void DrawLineVec2(float x1, float y1, float x2, float y2, Vec4 color)
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
    
    float color_start[4] =
    {
        color.x,
        color.y,
        color.z,
        color.w
    };

    float color_end[4] =
    {
        color.x,
        color.y,
        color.z,
        color.w
    };

    Draw2DLine(start, end, color_start, color_end);
}

static inline __attribute__((always_inline))
void DrawBox2D( float x, float y, float width, float height, Vec4 color)
{
    DrawLineVec2(x,         y,          x + width, y,          color);
    DrawLineVec2(x + width, y,          x + width, y + height, color);
    DrawLineVec2(x + width, y + height, x,         y + height, color);
    DrawLineVec2(x,         y + height, x,         y,          color);
}

static inline __attribute__((always_inline))
void DrawLineVec3(Vec3 a, Vec3 b, Vec3 color)
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
        color.x,
        color.y,
        color.z,
        1.0f
    };

    float color_end[4] =
    {
        color.x,
        color.y,
        color.z,
        1.0f
    };

    RenderLineWorld(start, end, color_start, color_end);
}

static inline __attribute__((always_inline))
void DrawBoundingBox(CNode* node)
{
    Vec3 min = node->m_bounds.m_min;
    Vec3 max = node->m_bounds.m_max;

    // Local-space AABB corners
    Vec3 p0 = { min.x, min.y, min.z };
    Vec3 p1 = { max.x, min.y, min.z };
    Vec3 p2 = { max.x, max.y, min.z };
    Vec3 p3 = { min.x, max.y, min.z };

    Vec3 p4 = { min.x, min.y, max.z };
    Vec3 p5 = { max.x, min.y, max.z };
    Vec3 p6 = { max.x, max.y, max.z };
    Vec3 p7 = { min.x, max.y, max.z };

    // Transform into world space
    p0 = TransformPoint(&node->m_mtx, p0);
    p1 = TransformPoint(&node->m_mtx, p1);
    p2 = TransformPoint(&node->m_mtx, p2);
    p3 = TransformPoint(&node->m_mtx, p3);

    p4 = TransformPoint(&node->m_mtx, p4);
    p5 = TransformPoint(&node->m_mtx, p5);
    p6 = TransformPoint(&node->m_mtx, p6);
    p7 = TransformPoint(&node->m_mtx, p7);

    // Bottom
    DrawLineVec3(p0, p1, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineVec3(p1, p2, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineVec3(p2, p3, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineVec3(p3, p0, (Vec3){ 1.0f, 1.0f, 1.0f });

    // Top
    DrawLineVec3(p4, p5, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineVec3(p5, p6, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineVec3(p6, p7, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineVec3(p7, p4, (Vec3){ 1.0f, 1.0f, 1.0f });

    // Vertical edges
    DrawLineVec3(p0, p4, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineVec3(p1, p5, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineVec3(p2, p6, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineVec3(p3, p7, (Vec3){ 1.0f, 1.0f, 1.0f });
}

static inline __attribute__((always_inline))
Vec3 GetBoneModelPosition(CZBodyPart* bone)
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

static inline __attribute__((always_inline))
Vec3 GetBoneWorldPosition(CZSealBody* entity, CZBodyPart* bone)
{
    Vec3 modelPosition = GetBoneModelPosition(bone);

    return TransformPoint(
        &entity->pNode->m_mtx,
        modelPosition
    );
}

static inline __attribute__((always_inline))
bool GetBoneWorldPosByIndex(CZSealBody* entity, FT_BONE idx, Vec3* wsOrigin)
{
    if (entity == 0 || wsOrigin == 0)
        return;

    CZBodyPart* bone = entity->mSkeleton[idx];
    if (!bone)
        return;

    *wsOrigin = GetBoneWorldPosition(entity, bone);

    return true;
}

static inline __attribute__((always_inline))
void DebugDrawSkeleton(CZSealBody* entity)
{
    if (entity == 0 || entity->pNode == 0)
        return;

    for (int i = 0; i < 33; i++)
    {
        CZBodyPart* bone = entity->mSkeleton[i];

        if (bone == 0)
            continue;

        CZBodyPart* parent = bone->pParentBone;

        if (parent == 0)
            continue;

        Vec3 bonePosition =
            GetBoneWorldPosition(entity, bone);

        Vec3 parentPosition =
            GetBoneWorldPosition(entity, parent);

		// Project the 3D positions to 2D screen space and draw the line
		Vec2 screen, screen_2;
		if (WorldToScreen(bonePosition, &screen) && WorldToScreen(parentPosition, &screen_2))
			DrawLineVec2(screen.x, screen.y, screen_2.x, screen_2.y, (Vec4){ 1.0f, 1.0f, 1.0f, 1.0f });

		// Draw the 3D line as well
        DrawLineVec3(
            bonePosition,
            parentPosition,
            (Vec3){ 1.0f, 1.0f, 1.0f }
        );
    }
}

static inline __attribute__((always_inline))
void DrawSkeleton(CZSealBody* entity)
{
    if (entity == 0 || entity->pNode == 0)
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

            CZBodyPart* bone =
                entity->mSkeleton[boneIndex];

            if (bone == 0)
            {
                havePrevious = 0;
                continue;
            }

            Vec3 currentPosition =
                GetBoneWorldPosition(entity, bone);

            if (!havePrevious)
            {
                previousPosition = currentPosition;
                havePrevious = 1;
                continue;
            }

            Vec2 currentScreen;
            Vec2 previousScreen;

            if (WorldToScreen(currentPosition, &currentScreen) &&
                WorldToScreen(previousPosition, &previousScreen))
            {
                DrawLineVec2(
                    previousScreen.x,
                    previousScreen.y,
                    currentScreen.x,
                    currentScreen.y,
                    (Vec4){ 1.0f, 1.0f, 1.0f, 1.0f }
                );
            }

            previousPosition = currentPosition;
        }
    }
}

static inline __attribute__((always_inline))
void DrawStringTest(C2DString* string, C2DFont* font, void* camera, s32 x, s32 y)
{
	C2DString_Load(string, "HELLO WORLD", font, x, y);
	C2DString_Draw(string, camera);
}

bool IsVisible(CZSeal* fromEntity, CZSeal* toEntity)
{
    if (fromEntity == 0 || toEntity == 0 || fromEntity->mTargetCount <= 0 || fromEntity->pTargetArray == 0)
        return false;

    for ( int i = 0; i < fromEntity->mTargetCount; i++)
    {
        CTarget& pTarget = fromEntity->pTargetArray[i];

        if (pTarget.pEntity == toEntity)
            return pTarget.m_visible;
    }

    return false;
}

// ------------------------------------------------------------
// Native Hook
// ------------------------------------------------------------
// CCameraApp::Tick -> CZSealBody::CheckDIShoot
__attribute__((noinline))
void hk_CheckDIShoot(CZSealBody* seal, s64 a2, int a3)
{
	// execute the original method 
    CheckDIShoot(seal, a2, a3);

    if (seal->mHealth <= 0.0f)
        return;


    CZSealBody* pTargetSeal = 0;
    f32 bestTargetDistSq = 99999999.0f;
	
    // esp
	{
		ZArray* sealArray = (ZArray*)gSealArray;
		if (seal == 0 || sealArray == 0 || 
    	    sealArray->count == 0 || 
    	    sealArray->begin == 0 || 
    	    sealArray->end == 0)
    	        return;
		
    	ZIterator* it = (ZIterator*)sealArray->begin;
    	if (it == 0)
    	    return;

    	ZIterator* end = (ZIterator*)it->prev;
    	if (end == 0)
    	    return;

    	do
    	{
    	    // skip if entity is null or entity is the same as seal
    	    CZSealBody* entity = (CZSealBody*)it->data;
    	    if (entity == 0 || entity == seal)
    	    {
    	        it = (ZIterator*)it->next;
    	        continue;
    	    }

    	    // check if entity is rendered
    	    CNode* pNode = (CNode*)entity->pNode;
    	    if (pNode == 0 || CNode_Rendered(pNode) == 0) // CNode_Rendered sometimes fails and causes unrendered entities to show up which looks like flickering
    	    {
    	        it = (ZIterator*)it->next;
    	        continue;
    	    }

    	    //  skip if entity is same team as seal or is dead 
    	    bool isSealTeam = (entity->mTeamID == 0x84000006 || entity->mTeamID == 0x8400000A);
    	    if (isSealTeam || entity->mTeamID == seal->mTeamID || entity->mHealth <= 0.0f)
    	    {
    	        it = (ZIterator*)it->next;
    	        continue;
    	    }

    	    DrawBoundingBox(pNode);
    	    DrawSkeleton(entity);


            // @todo: find target for aimbot
            Vec2 screen;
            Vec3 wsBoneHead;
            if (IsVisible(seal, entity) && GetBoneWorldPosByIndex(entity, FT_BONE, &wsBoneHead) && WorldToScreen(wsBoneHead, &screen))
            {    
                f32 dx = screen.x - 320.0f;
                f32 dy = screen.y - 224.0f;
                f32 aimDistSq = dx * dx + dy * dy;
                if (aimDistSq < bestTargetDistSq)
                {
                    bestTargetDistSq = distSq;
                    pTargetSeal = entity;
                }
            }

    	    it = (ZIterator*)it->next;
    	}
    	while (it && it->data != end->data);
	}

    // infinite ammo
    {
        CZKit* kit = &seal->mKit;
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
                    for (int j = 0; j < pWeapon->defaultMags; j++)
                        kit->mPrimaryMags[j] = newAmmo;
                }
                
                case 1:
                {
                    for (int j = 0; j < pWeapon->defaultMags; j++)
                        kit->mSecondaryMags[j] = newAmmo;
                }
                
                case 2: kit->mEqSlot1Ammo = newAmmo;
                case 3: kit->mEqSlot2Ammo = newAmmo;
                case 4: kit->mEqSlot3Ammo = newAmmo;
            }
        }
    }

    // perfect shot
    {
        seal->mShoulderRecoil = 0.0f;
        
        CZKit* kit = &seal->mKit;
        kit->mRecoilPunch = (Vec2){ 0.0f, 0.0f };
        kit->mPrevRecoilPunch = (Vec2){ 0.0f, 0.0f };
        kit->mRifleKick = (Vec3){ 0.0f, 0.0f, 0.0f };
        //  kit->mScreenOffset = {0.0f, 0.0f};
    }

    // aimbot
    {
        Vec3 targetOrigin;
        if (pTargetSeal && GetBoneWorldPosByIndex(pTargetSeal, FT_BONE_head, &targetOrigin))
        {
            seal->mReticlePoint = targetOrigin;

            Vec2 screen;
            if (WorldToScreen(targetOrigin, &screen))
            {
                float start[4] {320.f, 224.f, 0.f, 1.f};
                float end[4]{screen.x, screen.y, 0.0f, 1.0f};
                float color_start[4]{1.0f, 1.0f, 1.0f, 0.3f};
                float color_end[4]{1.f, 0.0f, 0.0f, 0.75f};
                Draw2DLine(start, end, color_start, color_end);
            }
        }
    }
}