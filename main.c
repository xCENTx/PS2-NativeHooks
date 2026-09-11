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

static Vec3 GetBoneWorldPosition(CZSealBody* entity, CZBodyPart* bone)
{
    Vec3 modelPosition = GetBoneModelPosition(bone);

    return TransformPoint(
        &entity->pNode->m_mtx,
        modelPosition
    );
}

static bool GetBoneWorldPosByIndex(CZSealBody* entity, FT_BONE idx, Vec3* wsOrigin)
{
    if (entity == 0 || wsOrigin == 0)
        return false;

    CZBodyPart* bone = entity->mSkeleton[idx];
    if (!bone)
        return false;

    *wsOrigin = GetBoneWorldPosition(entity, bone);

    return true;
}

static bool IsVisible(CZSealBody* fromEntity, CZSealBody* toEntity)
{
    if (fromEntity == 0 || toEntity == 0 || fromEntity->mTargetCount <= 0 || fromEntity->pTargetArray == 0)
        return false;

    for ( int i = 0; i < fromEntity->mTargetCount; i++)
    {
        CTarget* pTarget = &fromEntity->pTargetArray[i];

        if (pTarget->pEntity == toEntity)
            return pTarget->m_visible;
    }

    return false;
}

// ------------------------------------------------------------
// Draw Primitives
// ------------------------------------------------------------

// invokes Draw2DLine
static void DrawLineCanvas(float x1, float y1, float x2, float y2, Vec4 color)
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

static void DrawBox2D( float x, float y, float width, float height, Vec4 color)
{
    DrawLineCanvas(x,         y,          x + width, y,          color);
    DrawLineCanvas(x + width, y,          x + width, y + height, color);
    DrawLineCanvas(x + width, y + height, x,         y + height, color);
    DrawLineCanvas(x,         y + height, x,         y,          color);
}

// invokes RenderLineWorld
static void DrawLineWorld(Vec3 a, Vec3 b, Vec3 color)
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

static void DrawStringTest(C2DString* string, C2DFont* font, void* camera, s32 x, s32 y)
{
	C2DString_Load(string, "HELLO WORLD", font, x, y);
	C2DString_Draw(string, camera);
}

// rewrite of Draw2DLine
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

        packet->vertices[v].xyz =
            MakeGSVertex( outer_x, outer_y );

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

// draws the bounds of a node in world space
static void DebugDrawBoundingBoxWorld(CNode* node)
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
    DrawLineWorld(p0, p1, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineWorld(p1, p2, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineWorld(p2, p3, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineWorld(p3, p0, (Vec3){ 1.0f, 1.0f, 1.0f });

    // Top
    DrawLineWorld(p4, p5, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineWorld(p5, p6, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineWorld(p6, p7, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineWorld(p7, p4, (Vec3){ 1.0f, 1.0f, 1.0f });

    // Vertical edges
    DrawLineWorld(p0, p4, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineWorld(p1, p5, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineWorld(p2, p6, (Vec3){ 1.0f, 1.0f, 1.0f });
    DrawLineWorld(p3, p7, (Vec3){ 1.0f, 1.0f, 1.0f });
}

// connects all skeleton points on a czseal and draws in both world and canvas spaces
static void DebugDrawSkeleton(CZSealBody* entity)
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
			DrawLineCanvas(screen.x, screen.y, screen_2.x, screen_2.y, (Vec4){ 1.0f, 1.0f, 1.0f, 1.0f });

		// Draw the 3D line as well
        DrawLineWorld(
            bonePosition,
            parentPosition,
            (Vec3){ 1.0f, 1.0f, 1.0f }
        );
    }
}

// draws a player skeleton using custom bone indexing
static void DrawSkeleton(CZSealBody* entity)
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
                DrawSmooth2DLineNative(
                    previousScreen.x,
                    previousScreen.y,
                    currentScreen.x,
                    currentScreen.y,
                    0.5f,
                    (Vec4){ 1.0f, 1.0f, 1.0f, 1.0f },
                    (Vec4){ 1.0f, 1.0f, 1.0f, 1.0f }
                );
            }

            previousPosition = currentPosition;
        }
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
static void ProcessPlayerESP(void* obj, void* ctx)
{
    CZSealBody* entity;
    ctxPlayerESP* esp;
    CNode* pNode;
    bool isSealTeam;
    f32 dx;
    f32 dy;
    f32 aimDistSq;

    entity = (CZSealBody*)obj;
    esp = (ctxPlayerESP*)ctx;

    if (entity == 0 || esp == 0 || esp->seal == 0)
        return;

    if (entity == esp->seal)
        return;

    pNode = (CNode*)entity->pNode;

    if (pNode == 0 || CNode_Rendered(pNode) == 0)
        return;

    isSealTeam = entity->mTeamID == 0x84000006 || entity->mTeamID == 0x8400000A;

    if (isSealTeam || entity->mTeamID == esp->seal->mTeamID || entity->mHealth <= 0.0f)
        return;
    
    //  DebugDrawBoundingBoxWorld(pNode);
    DrawSkeleton(entity);

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
    
    DebugDrawBoundingBoxWorld(pNode);

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

// ------------------------------------------------------------
// Native Hook
// ------------------------------------------------------------
// CCameraApp::Tick -> CZSealBody::CheckDIShoot
__attribute__((section(".hook"), noinline))
void hk_CheckDIShoot(CZSealBody* seal, s64 a2, int a3)
{
	// execute the original method 
    CheckDIShoot(seal, a2, a3);

    if (seal == 0)
        return;
    
    // ESP
    ctxPlayerESP ctxPlayers;
    ctxPickupESP ctxPickups;
    {
        ctxPlayers.seal = seal;
        ctxPlayers.target = 0;
        ctxPlayers.bestTargetDistSq = 99999999.0f;
        ZArray_ForEach((ZArray*)gSealArray, ProcessPlayerESP, &ctxPlayers);

        ctxPickups.seal = seal;
        ctxPickups.target = 0;
        ctxPickups.bestTargetDistSq = 99999999.0f;
        ZArray_ForEach((ZArray*)gPickupArray, ProcessPickupESP, &ctxPickups);
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
                    break;
                }
                
                case 1:
                {
                    for (int j = 0; j < pWeapon->defaultMags; j++)
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
    {
        seal->mShoulderRecoil = 0.0f;
        
        CZKit* kit = &seal->mKit;
        kit->mRecoilPunch = (Vec2){ 0.0f, 0.0f };
        kit->mPrevRecoilPunch = (Vec2){ 0.0f, 0.0f };
        kit->mRifleKick = (Vec3){ 0.0f, 0.0f, 0.0f };
        //  kit->mScreenOffset = {0.0f, 0.0f};
    }

    // aimbot
    if (ctxPlayers.target != 0)
    {   
        Vec2 screen[2];
        Vec3 targetOrigin[2];
        if (ctxPlayers.target && GetBoneWorldPosByIndex(ctxPlayers.target, FT_BONE_head, &targetOrigin[0]) && WorldToScreen(targetOrigin[0], &screen[0])
            && GetBoneWorldPosByIndex(seal, FT_BONE_rhand, &targetOrigin[1]) && WorldToScreen(targetOrigin[1], &screen[1]))
        {
            seal->mReticlePt = targetOrigin[0];    
            seal->mAimPoint = targetOrigin[0];    
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