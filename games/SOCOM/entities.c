#include "entities.h"
#include "math.h"

bool IsAlive(CZSealBody* seal)
{
    return seal && (seal->m_ent.m_EntityBits & ENTITY_IS_ALIVE);
}

bool IsCharacter(CEntity* entity)
{
    if (!entity)
        return false;

    CNode* node = entity->p_Node;
    
    return node 
        && (node->mBits & CNODE_CHARACTER) != 0 
        && entity->mEntityType == ENTITY_SEAL;
}

bool IsVisible(CZSealBody* fromSeal, CZSealBody* toSeal)
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

bool IsAlphaUnit(CZSealBody* seal)
{
    u32 mask = seal->m_ent.m_TeamMask;

    return (mask & FIRETEAM_MASK(FT_FIRETEAM)) &&
           (mask & FIRETEAM_MASK(FT_ALPHA));
}

bool IsBravoUnit(CZSealBody* seal)
{
    u32 mask = seal->m_ent.m_TeamMask;

    return (mask & FIRETEAM_MASK(FT_FIRETEAM)) &&
           (mask & FIRETEAM_MASK(FT_BRAVO));
}

bool IsSealTeamUnit(CZSealBody* seal)
{
    u32 mask = seal->m_ent.m_TeamMask;

    return (mask & FIRETEAM_MASK(FT_FIRETEAM)) && (mask & (FIRETEAM_MASK(FT_ALPHA) | FIRETEAM_MASK(FT_BRAVO)));
}

bool SealJoinFireteam(CZSealBody* seal, u32 team)
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

Vec3 GetBoneWorldPosition(CZSealBody* seal, CZBodyPart* bone)
{
    Vec3 modelPosition = GetBoneModelPosition(bone);

    return TransformPoint(
        &seal->m_ent.p_Node->m_mtx,
        modelPosition
    );
}

bool GetBoneWorldPosByIndex(CZSealBody* seal, FT_BONE idx, Vec3* wsOrigin)
{
    if (seal == 0 || wsOrigin == 0)
        return false;

    CZBodyPart* bone = seal->m_Skeleton[idx];
    if (!bone)
        return false;

    *wsOrigin = GetBoneWorldPosition(seal, bone);

    return true;
}

bool GetMuzzleWorldLocation(CZSealBody* seal, Vec3* wsOrigin)
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

bool RespawnSeal(CZSealBody* seal)
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

    return true;
}

bool RespawnLocalPlayer()
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

CZSealBody* SpawnAIBot(CCharacterType* character, const char* name, u32 teamMask, Matrix4x4 position, u32* outId)
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

CZSealBody* SpawnFriendlyAIBot(const char* name, u32* outId)
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

bool RemoveBot(u8 id)
{

}
