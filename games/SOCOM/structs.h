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

typedef struct C2DFont C2DFont;
typedef struct C2DString C2DString;
typedef struct CNode CNode;
typedef struct CZCamera CZCamera;
typedef struct CZBodyPart CZBodyPart;
typedef struct CZSealBody CZSealBody;

// ------------------------------------------------------------
// Enums
// ------------------------------------------------------------

typedef s8 RDR_COMMAND;

enum
{
	RDR_AFTER_ERROR_REBOOT,
	RDR_AFTER_REBOOT,
    RDR_EXIT,
    RDR_INTRO,
    RDR_LOAD,
    RDR_MP_FINAL,
    RDR_MP_ROUND,
    RDR_NET_ABANDONED,
    RDR_NET_ABORT,
    RDR_NET_ERROR,
    RDR_RETURN_FROM_NET_GUI
};

typedef s8 LAUNCH_COMMAND;

enum
{
    LAUNCH_CMD_m,   // main
};

typedef s8 LAUNCH_FLAG;

enum
{
    LAUNCH_FLAGS_NONE
};


typedef s8 FT_COMMAND;

enum
{
	unknown = 0,
	FT_RETICULE_ACTION = 1,
	FT_DEPLOY = 2,
	FT_FIRE_AT_WILL = 3
};

typedef s8 FT_FIRETEAM;

enum
{
	FT_MP_PLAYER = 0,
	FT_FIRETEAM_ = 1,
	FT_ALPHA = 2,
	FT_BRAVO = 3,
	FT_USER = 4,
	FT_RESERVED = 5,
	FT_ESCORTEE = 6,
	FT_MP_RADIO = 7,
	FT_MP_SOP = 8,
	FT_NEUTRAL = 9,
	FT_TURRETS = 10,
	FT_HOSTAGE = 11,
	FT_SUPPORT = 12,
	FT_ENEMIES = 13,
	FT_ALLIES = 14,
	FT_ALL = 15
};

typedef s8 FT_BONE;

enum
{
    FT_BONE_MIN = 0,
    FT_BONE_root = FT_BONE_MIN,
	FT_BONE_aimnodes,
	FT_BONE_lfoot,			//	left heel
	FT_BONE_rfoot,			//	right heel
	FT_BONE_lhand,			//	left hand
	FT_BONE_spinelo,			//	lower spine
	FT_BONE_rhand,			//	right hand
	FT_BONE_hips,
	FT_BONE_head,			//	center head
	FT_BONE_neck,			//	neck
	FT_BONE_spinehi,			//	upper spine
	FT_BONE_lthigh,			//	left hip
	FT_BONE_rthigh,			//	right hip
	FT_BONE_rcalf,			//	right knee
	FT_BONE_rbicep,			//	right shoulder
	FT_BONE_rforearm,		//	right elbow
	FT_BONE_lbicep,			//	left shoulder
	FT_BONE_lforearm,		//	left elbow
	FT_BONE_lscap,
	FT_BONE_rscap,
	FT_BONE_lshoulder_wgt,	//	left shoulder
	FT_BONE_rshoulder_wgt,	//	right shoulder
	FT_BONE_lcalf,			//	knee
	FT_BONE_ltoe,			//	foot	
	FT_BONE_rtoe,			//	foot
	FT_BONE_weapon,
	FT_BONE_rifle,
	FT_BONE_pistol,
	FT_BONE_grenade,
	FT_BONE_reyeball,
	FT_BONE_leyeball,
	FT_BONE_reyelid,
	FT_BONE_leyelid,
	FT_BONE_MAX = FT_BONE_leyelid
};

typedef s8 WP_ENCUMBRANCE;
enum
{
	ENCUMBRANCE_LIGHT,
	ENCUMBRANCE_MEDIUM,
	ENCUMBRANCE_HEAVY,
	ENCUMBRANCE_VERY_HEAVY,
	ENCUMBRANCE_NOT_ENCUMBERED,
	ENCUMBRANCE_NUM_ECUMBTYPES
};

typedef s32 WP_FIREMODE;
enum
{
	FIREMODE_SAFETY,
	FIREMODE_SINGLE,
	FIREMODE_BURST,
	FIREMODE_AUTOFIRE,
	FIREMODE_SPECIAL_MODE,
	FIREMODE_NUM_FIREMODES
};

typedef s8 WP_PROJECTILE_TYPE;
enum
{
	PROJECTILE_TYPE_NORMAL,
	PROJECTILE_TYPE_ONE_FRAME,
	PROJECTILE_TYPE_ONE_FRAME_SHOTGUN
};

typedef s8 WP_PROJECTILE_STATE;
enum
{
	PROJECTILE_STATE_EXPIRED,
	PROJECTILE_STATE_FLYOUT,
	PROJECTILE_STATE_AT_REST,
	PROJECTILE_STATE_TO_BE_DETONATED,
	PROJECTILE_STATE_DETONATION_TO_BE_HANDLED,
	PROJECTILE_STATE_WAS_DETONATED,
	PROJECTILE_STATE_TO_BE_REMOVED
};

typedef s8 WP_GRENADE_STATE;
enum
{
	GRENADE_STATE_CREATE,
	GRENADE_STATE_NEWPOS,
	GRENADE_STATE_DETONATE,
	GRENADE_STATE_REMOVE
};

// ------------------------------------------------------------
// Math
// ------------------------------------------------------------

typedef struct
{
    f32 x, y;
} Vec2;
static_assert(sizeof(Vec2) == 0x8, "Size of Vec2 is not correct.");

typedef struct
{
    f32 x, y, z;
} Vec3;
static_assert(sizeof(Vec3) == 0xC, "Size of Vec3 is not correct.");

typedef struct
{
    f32 x, y, z, w;
} Vec4;
static_assert(sizeof(Vec4) == 0x10, "Size of Vec4 is not correct.");

typedef struct
{
    f32 m[4][4];
} Matrix4x4;
static_assert(sizeof(Matrix4x4) == 0x40, "Size of Matrix4x4 is not correct.");

typedef struct
{
	Vec3 m_min;
	Vec3 m_max;
} AABB;
static_assert(sizeof(AABB) == 0x18, "Size of AABB is not correct.");

typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} IPNT3D;
static_assert(sizeof(IPNT3D) == 0xC, "Size of IPNT3D is not correct.");

typedef struct 
{
	s32 left; //0x0000
	s32 top; //0x0004
	s32 right; //0x0008
	s32 bottom; //0x000C
} tag_RECT; //Size: 0x0010
static_assert(sizeof(tag_RECT) == 0x10, "Size of tag_RECT is not correct.");

// ------------------------------------------------------------
// Drawing Primitives
// ------------------------------------------------------------
#define GS_ALPHA_NORMAL 0x44ULL
#define GS_REG_ALPHA_1 0x42
#define GS_REG_AD      0x0E
#define CIRCLE_SEGMENTS 64
#define CIRCLE_VERTICES ((CIRCLE_SEGMENTS + 1) * 2)

typedef struct
{
    u32 r;
    u32 g;
    u32 b;
    u32 a;
} GSRGBAQ;

typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    s32 w;
} GSXYZ2;

typedef struct
{
    GSRGBAQ rgba;
    GSXYZ2 xyz;
} GSPackedVertex;

typedef struct __attribute__((aligned(16)))
{
    s32 dma[4];
    u64 gif_tag;
    u64 gif_regs;
    GSPackedVertex vertices[2];
} GSLinePacket;

typedef struct __attribute__((aligned(16)))
{
    s32 dma[4];
    u64 gif_tag;
    u64 gif_regs;
    GSPackedVertex vertices[4];
} GSLineStripPacket;

typedef struct __attribute__((aligned(16)))
{
    s32 dma[4];
    u64 gif_tag;
    u64 gif_regs;
    GSPackedVertex vertices[8];
} GSFeatherLinePacket;

typedef struct __attribute__((aligned(16)))
{
    s32 dma[4];
    u64 gif_tag;
    u64 gif_regs;
    GSPackedVertex vertices[CIRCLE_VERTICES];
} GSCirclePacket;

// ------------------------------------------------------------
// Z containers
// ------------------------------------------------------------

typedef struct
{
	u32 next;
	u32 prev;
	u32 data;
} ZIterator;
static_assert(sizeof(ZIterator) == 0xC, "Size of ZIterator is not correct.");

typedef struct
{
	s32 count;
	u32 begin;
	u32 end;
} ZArray;
static_assert(sizeof(ZArray) == 0xC, "Size of ZArray is not correct.");


// ------------------------------------------------------------
// Camera
// ------------------------------------------------------------

typedef struct
{
	Vec4 m_quat; //0x0000
	Vec4 m_fog_color; //0x0010
	f32 m_hfov; //0x0020
	f32 m_vfov; //0x0024
	f32 m_near_plane; //0x0028
	f32 m_mid_plane; //0x002C
	f32 m_far_plane; //0x0030
	Vec3 m_px_fog_color; //0x0034
	Vec3 m_nx_fog_color; //0x0040
	Vec3 m_pz_fog_color; //0x004C
	Vec3 m_nz_fog_color; //0x0058
	f32 m_fog_near; //0x0064
	f32 m_fog_far; //0x0068
	f32 m_fog_mid; //0x006C
	f32 m_fogA; //0x0070
	f32 m_fogB; //0x0074
	f32 m_fog_density; //0x0078
	f32 m_fog_top; //0x007C
	f32 m_fog_bottom; //0x0080
	f32 m_landmark_fog_top; //0x0084
	f32 m_landmark_fog_bottom; //0x0088
	u32 m_fog_flags; //0x008C
} tag_CAMERA_PARAMS; //Size: 0x0090
static_assert(sizeof(tag_CAMERA_PARAMS) == 0x90, "Size of tag_CAMERA_PARAMS is not correct.");

typedef struct
{
	Matrix4x4 mtxWorldToView; //0x0000
	Matrix4x4 mtxWorldToClip; //0x0040
	Matrix4x4 mtxViewToClip; //0x0080
	Matrix4x4 mtxViewToScreen; //0x00C0
} tag_ZCAM_MTX_SET; //Size: 0x0100
static_assert(sizeof(tag_ZCAM_MTX_SET) == 0x100, "Size of tag_ZCAM_MTX_SET is not correct.");

typedef struct
{
	Vec4 ViewPlanePoint; //0x0000
	Vec4 NearPlanePoint; //0x0010
	Vec4 NearPlaneNormal; //0x0020
	Vec4 LeftPlaneNormal; //0x0030
	Vec4 RightPlaneNormal; //0x0040
	Vec4 TopPlaneNormal; //0x0050
	Vec4 BottomPlaneNormal; //0x0060
} ZCAM_CLIP_DATA; //Size: 0x0070
static_assert(sizeof(ZCAM_CLIP_DATA) == 0x70, "Size of ZCAM_CLIP_DATA is not correct.");

// ------------------------------------------------------------
// C2D
// ------------------------------------------------------------

typedef struct
{
    char pad_0000[0x0C];

    u32* vtable;             // 0x0C

    char pad_0010[0x0C];

    u32* pFont;             // 0x1C  // confirmed used by C2DString::Draw/Load path

    char pad_0020[0x08];

    u32* pStringData;       // 0x28  // Draw requires non-null

    char pad_002C[0x10];

    f32 mCharWidth;        // 0x3C
    f32 mCharHeight;       // 0x40

    char pad_0044[0x08];

    f32 mScale;            // 0x4C

    char pad_0050[0x10];

    f32 field_60;          // 0x60  // compared against 128.0f

    char pad_0064[0x20];

    s32 mXFixed;             // 0x84  // x << 4
    s32 mYFixed;             // 0x88  // y << 4
} C2D;
static_assert(sizeof(C2D) == 0x8C, "Size of C2D is not correct.");

struct C2DFont
{
    char pad_0000[0x10];

    u32* pResource;         // 0x10

    char pad_0014[0x10];

    f32 field_24;          // 0x24
    f32 field_28;          // 0x28
    f32 field_2C;          // 0x2C
};
_Static_assert(sizeof(C2DFont) == 0x30, "Size of C2DFont is not correct.");

typedef struct
{
    u16 unknown00;           // 0x00
    u16 height;              // 0x02
} C2DFontMetrics;
static_assert(sizeof(C2DFontMetrics) == 0x4, "Size of C2DFontMetrics is not correct.");

typedef struct
{
    char pad_0000[0x0C];

    C2DFontMetrics* pMetrics;          // 0x0C
} C2DFontResource;
static_assert(sizeof(C2DFontResource) == 0x10, "Size of C2DFontResource is not correct.");

typedef struct
{
    char data[0x80];
} C2DBitmap;
static_assert(sizeof(C2DBitmap) == 0x80, "Size of C2DBitmap is not correct.");

struct C2DString
{
    char pad_0000[0x0C];

    u32* vtable;             // 0x0C

    char pad_0010[0x0C];

    C2DFont* pFont;             // 0x1C

    char pad_0020[0x08];

    u32* pStringData;       // 0x28

    char pad_002C[0x10];

    f32 mCharWidth;        // 0x3C
    f32 mCharHeight;       // 0x40

    char pad_0044[0x08];

    f32 mScale;            // 0x4C

    char pad_0050[0x10];

    f32 field_60;          // 0x60

    char pad_0064[0x20];

    s32 mXFixed;             // 0x84
    s32 mYFixed;             // 0x88

    char pad_008C[0x80];     // unknown remaining fields
};
static_assert(sizeof(C2DString) == 0x10C, "Size of C2DString is not correct.");

typedef struct
{
    C2DBitmap base;          // 0x000

    f32 mWidth;            // 0x080
    f32 mHeight;           // 0x084
    f32 mSpeed;            // 0x088

    IPNT3D mTextOffset;      // 0x08C

    s32 mTargetX;            // 0x098
    s32 mTargetY;            // 0x09C
    s32 mTargetX2;           // 0x0A0
    s32 mTargetY2;           // 0x0A4

    s32 mState2;             // 0x0A8
    s32 mState;              // 0x0AC

    f32 mTime;             // 0x0B0

    C2DString mText;         // 0x0B4

    C2DBitmap mGlowingBackground; // 0x1C0

    // remaining reversed fields later
    char pad_0240[0x188];
} C2DOrderItem;
static_assert(sizeof(C2DOrderItem) == 0x3C8, "Size of C2DOrderItem is not correct.");


// ------------------------------------------------------------
// Orders menu
// ------------------------------------------------------------

typedef struct
{
	u32* pText; //0x0000
	u32* pDisplayData; //0x0004
	u32 pData; //0x0008
} CSubMenu; //Size: 0x000C
static_assert(sizeof(CSubMenu) == 0xC);

typedef struct
{
	s32 unk0000; //0x0000
	u32* pInterface; //0x0004
	char pad_0008[48]; //0x0008
	s32 state38; //0x0038
} CSealUnit; //Size: 0x003C
static_assert(sizeof(CSealUnit) == 0x3C);

typedef struct
{
	char displayText[32]; //0x0000
	char recoText[32]; //0x0020
	char description[128]; //0x0040
	s32 recoWordId; //0x00C0
	FT_COMMAND command; //0x00C4
	char pad_00C5[3]; //0x00C5
	s32 teamMask; //0x00C8
	bool multiplayerFlag; //0x00CC
	char pad_00CD[3]; //0x00CD
	s32 subMenuCount; //0x00D0
	CSubMenu* pSubMenu; //0x00D4
	s32 displayIndex; //0x00D8
} CMD_TABLE; //Size: 0x00DC
static_assert(sizeof(CMD_TABLE) == 0xDC);

typedef struct
{
	char displayText[32]; //0x0000
	char recoText[32]; //0x0020
	s32 recoWordId; //0x0040
	u8 teamType; //0x0044
	char pad_0045[3]; //0x0045
	s32 tableIndex; //0x0048
	bool hasCMD; //0x004C
	char pad_004D[3]; //0x004D
	CSealUnit* pUnit; //0x0050
	char descriptionText[128]; //0x0054
} TEAM_TABLE; //Size: 0x00D4
static_assert(sizeof(TEAM_TABLE) == 0xD4);

typedef struct
{
	char pad_0000[4]; //0x0000
	f32 timers[3]; //0x0004
	char pad_0010[4]; //0x0010
	C2DOrderItem* pTeamItems; //0x0014
	C2DOrderItem* pCommandItems; //0x0018
	C2DOrderItem* pContextItems; //0x001C
	f32 collapseTimer; //0x0020
	bool bClosing; //0x0024
	char pad_0025[3]; //0x0025
	s32 itemCount[3]; //0x0028
	s32 selection[3]; //0x0034
	FT_FIRETEAM selectedTeamType; //0x0040
	char pad_0041[3]; //0x0041
	CSubMenu* pSelectedSubMenu; //0x0044
	CMD_TABLE* pCommand; //0x0048
	char pad_004C[44]; //0x004C
	s32 currentSelection; //0x0078
	s32 currentColumn; //0x007C
    C2DFont* pFont;                  // 0x80  <-- important
    char pad_0084[8];                // 0x84
} OrdersMenu; //Size: 0x008C
static_assert(sizeof(OrdersMenu) == 0x8C);

typedef struct
{
	char pad_0000[0x12BD0]; //0x0000
	OrdersMenu s_OrdersMenu; //0x12BD0
} CHUD; //Size: 0x12C5C
static_assert(sizeof(CHUD) == 0x12C5C);


// ------------------------------------------------------------
// CNode
// ------------------------------------------------------------

struct __attribute__((packed)) CNode
{
    Matrix4x4 m_mtx; //0x0000
	AABB m_bounds; //0x0040
	s32 mType; //0x0058
	u32 mBits; //0x005C
	char pad_0060[4]; //0x0060
	CNode* pParent; //0x0064
	char pad_0068[40]; //0x0068
	char* pName; //0x0090
	u32 pNodeEx; //0x0094 // class CNodeEx*
	char mGlobalLighting; //0x0098
	unsigned char m_frameRendered; //0x0099
	char pad_009A[2]; //0x009A
	f32 mOpacity; //0x009C
	char pad_00A0[4]; //0x00A0
	s32 mTickCount; //0x00A4
	char pad_00A8[8]; //0x00A8
	u32 pModel; //0x00B0 // class CModel*
	char* pModelName; //0x00B4
	char pad_00B8[8]; //0x00B8
};
static_assert(sizeof(CNode) == 0x00C0, "Size of CNode is not correct.");


// ------------------------------------------------------------
// Camera
// ------------------------------------------------------------

struct CZCamera
{
    CNode m_node; //0x0000
	tag_CAMERA_PARAMS m_camera_params; //0x00C0
	Vec3 mFrustrum[3]; //0x0150
	Vec3 mFullFrustrum[6]; //0x0174
	s32 mFullFrustrumPoints; //0x01BC
	Vec2 mSin; //0x01C0
	Vec2 mCos; //0x01C8
	Vec2 mTan; //0x01D0
	Vec2 mCot; //0x01D8
	char pad_01E0[176]; //0x01E0
	f32 m_scrZ; //0x0290
	char pad_0294[28]; //0x0294
	u32 m_AboveMaterial; //0x02B0
	f32 m_landmark_far_plane; //0x02B4
	f32 m_RangeScale; //0x02B8
	char pad_02BC[4]; //0x02BC
	tag_ZCAM_MTX_SET m_mtxSet; //0x02C0
	char pad_03C0[16]; //0x03C0
	ZCAM_CLIP_DATA m_ClipSet; //0x03D0
	Vec2 m_screenAspect; //0x0440
	Vec2 m_screenCenter; //0x0448
	Vec2 m_screenOffset; //0x0450
	tag_RECT m_screen; //0x0458
	Vec2 m_screenConstant; //0x0468
	f32 m_Zmin; //0x0470
	f32 m_Zmax; //0x0474
	char pad_0478[160]; //0x0478
}; //Size: 0x0518
static_assert(sizeof(CZCamera) == 0x518, "Size of CZCamera is not correct.");

typedef struct __attribute__((packed))
{
	char pad_0000[52]; //0x0000
	CZCamera* pCamera; //0x0034
	CNode* pTransforms; //0x0038
	CZSealBody* pAttachedPlayer; //0x003C
	CZBodyPart* pSkeletonRoot; //0x0040
	char pad_0044[12]; //0x0044
	Vec3 mOrigin; //0x0050
	char mCtrlView; //0x005C
	char pad_005D[51]; //0x005D
	char mSavePeek; //0x0090
	char mSaveView; //0x0091
	bool bAutoDeathCam; //0x0092
	bool bAllowDeathCamControl; //0x0093
	char mCamDeathState; //0x0094
} CAppCamera; //Size: 0x0095
static_assert(sizeof(CAppCamera) == 0x95, "Size of CAppCamera is not correct.");



// ------------------------------------------------------------
// Weapons / Ammo
// ------------------------------------------------------------

typedef struct
{
	char* pName; //0x0000
	char* pDisplayName; //0x0004
	f32 impact; //0x0008
	f32 stun; //0x000C
	f32 piercing; //0x0010
	f32 explosiveDMG; //0x0014
	f32 explosiveRAD; //0x0018
	u32 pHitAnim; //0x001C // CZAnim*
	char pad_0020[32]; //0x0020
} CZAmmo; //Size: 0x0040
static_assert(sizeof(CZAmmo) == 0x40, "Size of CZAmmo is not correct.");

typedef struct 
{
	char pad_0000[4]; //0x0000
	char* pName; //0x0004
	char* pDisplayName; //0x0008
	char* pTextureName; //0x000C
	char* pIconName; //0x0010
	char* pGearName; //0x0014
	char* pModelName; //0x0018
	char* pBulletImpactName; //0x001C
	WP_FIREMODE maxFireMode; //0x0020
	s32 szMags; //0x0024
	s32 defaultMags; //0x0028
	f32 mSoundRadius; //0x002C
	f32 mSoundRadiusSq; //0x0030
	char Encumberance; //0x0034
	char pad_0035[3]; //0x0035
	f32 mMaxRange; //0x0038
	f32 mEffectiveRange; //0x003C
	f32 mMuzzleVelocity; //0x0040
	f32 mImpactRadius; //0x0044
	f32 mFireWait; //0x0048
	f32 mReloadTime; //0x004C
	bool bReloadAfterShot; //0x0050
	char pad_0051[3]; //0x0051
	u32 mBits; //0x0054
	f32 mRangeMin; //0x0058
	f32 mRangeMax; //0x005C
	s32 mItemID; //0x0060
	u32 pHitAnim; //0x0064   // class CZAnim *
	u32 pFireAnim; //0x0068
	u32 pZoomFireAnim; //0x006C
	u32 pDefaultSpecialAnim; //0x0070
	u32 pSpecialMaterialAnim; //0x0074
	u32 pSound_Reload; //0x0078   // class CSnd *
	char* pSoundName_Reload; //0x007C
	char* pAnimName_Hit; //0x0080
	char* pAnimName_Fire; //0x0084
	char* pAnimName_DefaultSpecial; //0x0088
	char* pAnimName_SpecialMaterial; //0x008C
	char pad_0090[12]; //0x0090
	ZArray mLegalAmmoList; //0x009C
	bool bHasFireMode[4]; //0x00A8
} CZWeapon; //Size: 0x00AC
static_assert(sizeof(CZWeapon) == 0xAC, "Size of CZWeapon is not correct.");

typedef struct __attribute__((packed))
{
	f32 mLifetime; //0x0000
	CNode* pNode; //0x0004
	CZAmmo* pData; //0x0008
	char mType; //0x000C
	bool bLocked; //0x000D
	bool bSecondary; //0x000E
	char mUnused; //0x000F
	s16 mID; //0x0010
	s16 mNetID; //0x0012
	bool bVisibleByUnits; //0x0014
} CPickup; //Size: 0x0015
static_assert(sizeof(CPickup) == 0x15, "Size of CPickup is not correct.");

// ------------------------------------------------------------
// Seal
// ------------------------------------------------------------
typedef struct __attribute__((packed))
{
	CZSealBody* pEntity; //0x0000
	Vec3 mVec; //0x0004
	f32 mDistSq; //0x0010
	f32 mDist; //0x0014
	f32 mVisibility; //0x0018
	f32 mAware; //0x001C
	s32 mDiHandle; //0x0020
	u8 m_d_computed : 1;
	u8 m_known      : 1;
	u8 m_visible    : 1;
	u8 m_hostile    : 1;
	u8 m_targeted   : 1;
	u8 m_dirty_di   : 1;
	u8 m_unused     : 2;
	char pad_0025[3]; //0x0025
} CTarget; //Size: 0x0028
static_assert(sizeof(CTarget) == 0x28, "Size of CTarget is not correct.");

struct CZBodyPart
{
	Vec3 mOrigin; //0x0000
	CNode* pNode; //0x000C
	Vec4 mRotation; //0x0010
	CZBodyPart* pParentBone; //0x0020
	s16 mID; //0x0024
	char pad_0026[2]; //0x0026
} ; //Size: 0x0028
static_assert(sizeof(CZBodyPart) == 0x28, "Size of CZBodyPart is not correct.");

typedef struct __attribute__((packed))
{
	s32 mHeadshots; //0x0000
	s32 mHeadHits; //0x0004
	s32 mHits; //0x0008
	s32 mShotsFired; //0x000C
	s32 mKills; //0x0010
	s32 mWasHit; //0x0014
	s32 mDeaths; //0x0018
	s32 mHostages; //0x001C
	s32 mBasesBlown; //0x0020
	s32 mHostagesRescued; //0x0024
	f32 mAccuracy; //0x0028
	s32 mTimesSeen; //0x002C
	s32 mStealthKills; //0x0030
	s32 mRestrains; //0x0034
	s32 mGrenadesThrown; //0x0038
	s32 mCqcTakedowns; //0x003C
	s32 mPrimaryRoundsFired; //0x0040
	s32 mSecondaryRoundsFired; //0x0044
	s16 mMVPScore; //0x0048
	s32 mRoundsWon; //0x004A
	s32 mRoundsLost; //0x004E
} SSealStats; //Size: 0x0052
static_assert(sizeof(SSealStats) == 0x52, "Size of SSealStats is not correct.");

typedef struct
{
	u16 mBitField; //0x0000
	char pad_0002[22]; //0x0002
	Vec2 mRecoilPunch; //0x0018
	Vec2 mPrevRecoilPunch; //0x0020
	char pad_0028[8]; //0x0028
	Vec3 mRifleKick; //0x0030
	s32 mFireRifleKickState; //0x003C
	s32 mHeartbeatState; //0x0040
	Vec2 mHeartbeat; //0x0044
	s32 mCurItemReticule; //0x004C
	Vec2 mScreenOffset; //0x0050
	char pad_0058[132]; //0x0058
	CZWeapon* pWeapons[10]; //0x00DC
	char pad_0104[80]; //0x0104
	CZAmmo* pAmmoTypes[10]; //0x0154
	char pad_017C[80]; //0x017C
	s32 mPrimaryMags[10]; //0x01CC
	s32 mSecondaryMags[10]; //0x01F4
	s32 mEqSlot1Ammo; //0x021C
	char pad_0220[36]; //0x0220
	s32 mEqSlot2Ammo; //0x0244
	char pad_0248[36]; //0x0248
	s32 mEqSlot3Ammo; //0x026C
	char pad_0270[1036]; //0x0270
	s32 PrimaryMagIndex; //0x067C
	s32 SecondaryMagIndex; //0x0680
	char pad_0684[112]; //0x0684
	s32 mWeaponFireTypes[2]; //0x06F4
	char pad_06FC[272]; //0x06FC
	s32 mWeaponFireCount; //0x080C
	float mWeaponFireDelta; //0x0810
	s32 mCurrentWeaponIndex; //0x0814
	char pad_0818[4]; //0x0818
	s32 mMaxWeaponIndex; //0x081C
	u32 pSealBody; //0x0820 // CZSealBody*
	char pad_0824[108]; //0x0824
} CZKit; //Size: 0x0890
static_assert(sizeof(CZKit) == 0x890, "Size of CZKit is not correct.");

struct CZSealBody
{
	char pad_0000[16]; //0x0000
	char mEntityType; //0x0010
	char pad_0011[3]; //0x0011
	char* pName; //0x0014
	u32 m_UnitsSeenBy; //0x0018
	Vec3 mOrigin; //0x001C
	CNode* pNode; //0x0028
	Vec3 mVel_M; //0x002C
	Vec3 mVel_W; //0x0038
	Vec3 mVel_R; //0x0044
	Vec4 mQuat; //0x0050
	Vec3 mNextVel_W; //0x0060
	char pad_006C[4]; //0x006C
	Vec4 mNextQuat; //0x0070
	Matrix4x4 mMatrix; //0x0080
	u32 pSealCtrl; //0x00C0 // CZSealCtrl*
	s32 mTeamID; //0x00C4
	f32 mMaxTargetRange; //0x00C8
	s32 mMaxTargetCount; //0x00CC
	s32 mTargetCount; //0x00D0
	CTarget* pTargetArray; //0x00D4
	s32 mAwareCounter; //0x00D8
	u32 mEntityBits; //0x00DC
	char pad_00E0[128]; //0x00E0
	char mZoomIndex; //0x0160
	char mLastZoomIndex; //0x0161
	char pad_0162[2]; //0x0162
	f32 mZoomModifier; //0x0164
	char pad_0168[260]; //0x0168
	CZBodyPart* mSkeleton[33]; //0x026C
	char pad_02F0[20]; //0x02F0
	char mStance; //0x0304
	char mLean; //0x0305
	char pad_0306[2]; //0x0306
	f32 mShoulderRecoil; //0x0308
	char pad_030C[20]; //0x030C
	Vec3 mAimWorldPos; //0x0320
	char pad_032C[4]; //0x032C
	Vec3 mAimWorldAngles; //0x0330
	char pad_033C[236]; //0x033C
	Vec3 mRelativeRotation; //0x0428
	char pad_0434[156]; //0x0434
	SSealStats mSealStats; //0x04D0
	char pad_0522[14]; //0x0522
	CZKit mKit; //0x0530
	char pad_0DC0[40]; //0x0DC0
	CZSealBody* pCarry; //0x0DE8
	char pad_0DEC[220]; //0x0DEC
	f32 mElevation; //0x0EC8
	char pad_0ECC[4]; //0x0ECC
	f32 mHealth; //0x0ED0
	u32 m_deathType; //0x0ED4
	f32 m_time_of_death; //0x0ED8
	u32 m_killer_index; //0x0EDC
	u32 m_killer_weapon_index; //0x0EE0
	u32 m_lastkiller_id; //0x0EE4
	u32 m_ClientIndex; //0x0EE8
	u32 m_vote_tally; //0x0EEC
	char pad_0EF0[8]; //0x0EF0
	f32 m_movespeedmod; //0x0EF8
	f32 m_encumbmod; //0x0EFC
	f32 m_damagemod; //0x0F00
	f32 m_retmods; //0x0F04
	char pad_0F08[28]; //0x0F08
	Vec2 mHeadHealth; //0x0F24
	Vec2 mBodyHealth; //0x0F2C
	Vec2 mArmHealth_L; //0x0F34
	Vec2 mArmHealth_R; //0x0F3C
	Vec2 mLegHealth_L; //0x0F44
	Vec2 mLegHealth_R; //0x0F4C
	f32 mArmor[6]; //0x0F54
	char pad_0F6C[20]; //0x0F6C
	u32 m_bits_1; //0x0F80
	u32 m_bits_2; //0x0F84
	char pad_0F88[344]; //0x0F88
	Vec3 mAimDir; //0x10E0
	Vec3 mAimGoal; //0x10EC
	Vec3 mAimPoint; //0x10F8
	Vec3 mReticlePt; //0x1104
	Vec3 mAimNorm; //0x1110
	Vec3 mPrevAimPoint; //0x111C
	Vec3 mPrevAimNorm; //0x1128
	Vec3 mPrevReticlePt; //0x1134
	Vec3 mCurrAimPos; //0x1140
	char pad_114C[4]; //0x114C
	Vec3 mCurFirePos; //0x1150
	char pad_115C[4]; //0x115C
	Vec3 mSkeletonRoot; //0x1160
}; //Size: 0x116C
static_assert(sizeof(CZSealBody) == 0x116C, "Size of CZSealBody is not correct.");

#endif