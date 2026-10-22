
#include "bots.h"
#include "containers.h"
#include "entities.h"

static BotManager g_Bots = {0};

typedef struct
{
    CZSealBody* found;
    u32 entityId;
} BotLookup;

static void Bots_FindCallback(void* obj, void* context)
{
    BotLookup* lookup = (BotLookup*)context;
    CZSealBody* seal = (CZSealBody*)obj;

    if (!seal || !lookup || lookup->found)
        return;

    if (seal->m_ent.m_id == lookup->entityId)
        lookup->found = seal;
}

static CZSealBody* Bots_FindEntity(u32 entityId)
{
    BotLookup lookup = {0};

    lookup.entityId = entityId;

    ZArray_ForEach(
        (ZArray*)gSealArray,
        Bots_FindCallback,
        &lookup
    );

    return lookup.found;
}

static BotEntry* Bots_FindEntry(u32 entityId)
{
    for (u32 i = 0; i < BOT_MAX_COUNT; i++)
    {
        BotEntry* entry = &g_Bots.entries[i];

        if (entry->active && entry->entityId == entityId)
            return entry;
    }

    return NULL;
}

static BotEntry* Bots_FindFreeSlot(void)
{
    for (u32 i = 0; i < BOT_MAX_COUNT; i++)
    {
        BotEntry* entry = &g_Bots.entries[i];

        if (!entry->active)
            return entry;
    }

    return NULL;
}

static void Bots_ClearEntry(BotEntry* entry)
{
    if (!entry)
        return;

    entry->seal = NULL;
    entry->entityId = 0;
    entry->teamMask = 0;
    entry->active = false;
    entry->respawnPending = false;
}

void Bots_Init(void)
{
    Bots_Reset();
}

void Bots_Reset(void)
{
    for (u32 i = 0; i < BOT_MAX_COUNT; i++)
        Bots_ClearEntry(&g_Bots.entries[i]);

    g_Bots.count = 0;
    g_Bots.respawnsEnabled = false;
}

CZSealBody* Bots_Spawn( const char* name, s32 teamMask, const Matrix4x4* transform, const Vec3* position )
{
    if (!name || !*name)
        return NULL;

    CZSealBody* local_seal = ftsGetPlayer();
    if (!local_seal)
        return NULL;

    BotEntry* entry = Bots_FindFreeSlot();
    if (!entry)
        return NULL;

    CCharacterType* character = CCharacterType_GetByNameMod("UiCharType2");
    if (!character)
        return NULL;

    CZSealBody* bot = ftsCreateSeal(
        character,
        name,
        CTRL_AI
    );

    if (!bot)
        return NULL;

    if (bot->m_ent.p_Node)
        CNode_SetName(bot->m_ent.p_Node, name);

    CEntity_SetDisplayName(&bot->m_ent, name);
    CEntity_SetTeamMask(&bot->m_ent, teamMask);

    // Apply optional spawn overrides.
    if (transform)
    {
        CZSealBody_TeleportTo(bot, transform);
    }
    else if (position)
    {
        Matrix4x4 currentTransform = bot->m_ent.m_Matrix;
        currentTransform.m[3][0] = position->x;
        currentTransform.m[3][1] = position->y;
        currentTransform.m[3][2] = position->z;
        CZSealBody_TeleportTo(bot, &currentTransform);
    }
    else
    {

        Matrix4x4 currentTransform = bot->m_ent.m_Matrix;
        
        Vec3 lookDir = GetLookDirection(bot);

        currentTransform.m[3][0] = local_seal->m_ent.m_Matrix.m[3][0] + (lookDir.x * 5.f);
        currentTransform.m[3][1] = local_seal->m_ent.m_Matrix.m[3][1] + (lookDir.y * 5.f);
        currentTransform.m[3][2] = local_seal->m_ent.m_Matrix.m[3][2] + (lookDir.z * 5.f);

        CZSealBody_TeleportTo(bot, &currentTransform);
    }

    if (*(volatile u8*)gTheNetwork2)
    {
        // Register this AI as an additional network seal.
        s32 networkId = CZSealState_CreateLocalSeal(bot, 0);

        if (networkId >= 0)
        {
            bot->m_ent.m_id = (u32)networkId;

            // Initialize its replicated state.
            CZSealState_UpdateLocalSeal(bot);
        }
    }

    bot->m_should_respawn = 1;

    entry->seal = bot;
    entry->entityId = bot->m_ent.m_id;
    entry->teamMask = teamMask;
    entry->active = true;
    entry->respawnPending = false;

    g_Bots.count++;

    return bot;
}

bool Bots_SetTeam(u32 entityId, s32 teamMask)
{
    BotEntry* entry = Bots_FindEntry(entityId);

    if (!entry)
        return false;

    CZSealBody* bot = Bots_FindEntity(entityId);

    if (!bot)
        return false;

    CEntity_SetTeamMask(&bot->m_ent, teamMask);

    entry->seal = bot;
    entry->teamMask = teamMask;

    return true;
}

bool Bots_RequestRespawn(u32 entityId)
{
    BotEntry* entry = Bots_FindEntry(entityId);

    if (!entry)
        return false;

    entry->respawnPending = true;

    return true;
}

void Bots_SetRespawnsEnabled(bool enabled)
{
    g_Bots.respawnsEnabled = enabled;
}

void Bots_Tick(void)
{
    for (u32 i = 0; i < BOT_MAX_COUNT; i++)
    {
        BotEntry* entry = &g_Bots.entries[i];

        if (!entry->active)
            continue;

        CZSealBody* bot = Bots_FindEntity(entry->entityId);

        if (!bot)
        {
            Bots_ClearEntry(entry);

            if (g_Bots.count)
                g_Bots.count--;

            continue;
        }

        entry->seal = bot;

        if (!IsAlive(bot) &&
            (g_Bots.respawnsEnabled || entry->respawnPending))
        {
            bot->m_should_respawn = 1;
            entry->respawnPending = false;
        }
    }
}

u32 Bots_GetCount(void)
{
    return g_Bots.count;
}
