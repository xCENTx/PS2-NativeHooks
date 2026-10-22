
#ifndef SOCOM_BOTS_H
#define SOCOM_BOTS_H

#include "game.h"

#define BOT_NAME_LENGTH     32
#define BOT_MAX_COUNT       16
#define MAX_TEAM_PLAYERS    8
#define TEAM_SEALS          0x40000001u
#define TEAM_TERRORISTS     0x80000100u

typedef struct
{
    CZSealBody* seal;
    char name[BOT_NAME_LENGTH];
    u32 entityId;
    s32 teamMask;
    bool active;
    bool respawnPending;
} BotEntry;

typedef struct
{
    BotEntry entries[BOT_MAX_COUNT];
    u32 count;
    bool respawnsEnabled;
} BotManager;

void Bots_Init(void);
void Bots_Reset(void);
void Bots_Tick(void);

CZSealBody* Bots_Spawn(
    const char* name,
    s32 teamMask,
    const Matrix4x4* transform,
    const Vec3* position
);

void Bots_GenerateName(BotEntry* entry, const char* prefix);
bool Bots_SetTeam(u32 entityId, s32 teamMask);
bool Bots_RequestRespawn(u32 entityId);
void Bots_SetRespawnsEnabled(bool enabled);
u32 Bots_GetCount(void);
bool Bots_IsMultiplayerTeam(u32 teamMask);
u32 Bots_SelectSpawnTeam(u32 preferredTeam);
bool Bots_CanSpawnOnTeam(u32 teamMask, u32 teamCount);
u32 Bots_CountTeamMembers(u32 teamMask);

#endif