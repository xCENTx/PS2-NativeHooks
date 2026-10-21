
#ifndef SOCOM_BOTS_H
#define SOCOM_BOTS_H

#include "game.h"

#define BOT_MAX_COUNT 16

typedef struct
{
    CZSealBody* seal;
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
    const Matrix4x4* transform,
    s32 teamMask
);

bool Bots_SetTeam(u32 entityId, s32 teamMask);
bool Bots_RequestRespawn(u32 entityId);
void Bots_SetRespawnsEnabled(bool enabled);
u32 Bots_GetCount(void);

#endif
