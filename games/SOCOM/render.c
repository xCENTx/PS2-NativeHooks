#include "render.h"

#include "math.h"
#include "entities.h"

//
// ============================================================================
//  CONSTANTS
// ============================================================================
//

#define CIRCLE_SEGMENTS 64
#define CIRCLE_VERTICES ((CIRCLE_SEGMENTS + 1) * 2)

#define BONE_INVALID (-1)
#define BONE_CHAIN_COUNT 5 // (sizeof(BoneChains) / sizeof(BoneChains[0]))

#define GS_ALPHA_NORMAL 0x44ULL
#define GS_REG_ALPHA_1 0x42
#define GS_REG_AD      0x0E


//
// ============================================================================
//  STATIC DATA
// ============================================================================
//

static const s32 BoundingBoxEdges[12][2] =
{
    {0,1}, {1,2}, {2,3}, {3,0},
    {4,5}, {5,6}, {6,7}, {7,4},
    {0,4}, {1,5}, {2,6}, {3,7}
};

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


//
// ============================================================================
//  PRIVATE FUNCTIONS
// ============================================================================
//

static inline GSXYZ2 MakeGSVertex(f32 x, f32 y);
static inline void SetGSColor(GSRGBAQ* out, Vec4 color);

static void MakeBoundingBox(CNode* node, Vec3 world[8]);

static void spDrawLineGradient(float x1, float y1, float x2, float y2, Vec4 color_start, Vec4 color_end);
static void spDrawLineRGB(float x1, float y1, float x2, float y2, Vec3 color);

static void wsDrawLineGradient(Vec3 a, Vec3 b, Vec4 colorA, Vec4 colorB);
static void wsDrawLineRGB(Vec3 a, Vec3 b, Vec3 color);

static void DrawStringTest(C2DString* string, C2DFont* font, void* camera, s32 x, s32 y);
static void Draw2DLineNative(f32 x1, f32 y1, f32 x2, f32 y2, Vec4 color_start, Vec4 color_end);
static void Draw3DLineNative(Vec4 start, Vec4 end, Vec4 color_start, Vec4 color_end);
static void RenderLineWorldNative(Vec4 start, Vec4 end, Vec4 color_start, Vec4 color_end);
static void DrawSmooth2DLineNative( f32 x1, f32 y1, f32 x2, f32 y2, f32 width, Vec4 color_start, Vec4 color_end);
static void DrawFeathered2DLine( f32 x1, f32 y1, f32 x2, f32 y2, f32 width, f32 feather, Vec4 color_start, Vec4 color_end );


//
// ============================================================================
//  PROJECTION
// ============================================================================
//

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

//
// ============================================================================
//  2D DRAWING
// ============================================================================
//

void spDrawLine(f32 x1, f32 y1, f32 x2, f32 y2, Vec4 color)
{
    spDrawLineGradient(x1, y1, x2, y2, color, color);
}

void Draw2DCircle(f32 center_x, f32 center_y, f32 radius, f32 thickness, Vec4 color )
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


//
// ============================================================================
//  3D DRAWING
// ============================================================================
//

void wsDrawLine(Vec3 start, Vec3 end, Vec4 color)
{
    wsDrawLineGradient(start, end, color, color);
}


//
// ============================================================================
//  BOUNDING BOXES
// ============================================================================
//

void spDrawBoundingBox(CNode* node, Vec3 color)
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

void wsDrawBoundingBox(CNode* node, Vec3 color)
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


//
// ============================================================================
//  SKELETONS
// ============================================================================
//

void spDrawSkeleton(CZSealBody* seal)
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

void wsDrawSkeleton(CZSealBody* seal)
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

//
// ============================================================================
//  PRIVATE IMPLEMENTATION
// ============================================================================
//

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

static void spDrawLineRGB(f32 x1, f32 y1, f32 x2, f32 y2, Vec3 color)
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

static void wsDrawLineRGB(Vec3 start, Vec3 end, Vec3 color)
{
    wsDrawLine(start, end, (Vec4){ color.x, color.y, color.z, 1.0f});
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
