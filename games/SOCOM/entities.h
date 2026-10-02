#ifndef SOCOM_ENTITIES_H
#define SOCOM_ENTITIES_H

#include "game.h"

bool IsAlive(CZSealBody* seal);
bool IsCharacter(CEntity* entity);
bool IsVisible(CZSealBody* from, CZSealBody* to);

bool IsAlphaUnit(CZSealBody* seal);
bool IsBravoUnit(CZSealBody* seal);
bool IsSealTeamUnit(CZSealBody* seal);

bool SealJoinFireteam(CZSealBody* seal, u32 team);

Vec3 GetBoneModelPosition(CZBodyPart* bone);
Vec3 GetBoneWorldPosition(CZSealBody* seal, CZBodyPart* bone);
bool GetBoneWorldPosByIndex(CZSealBody* seal, FT_BONE bone, Vec3* position);
bool GetMuzzleWorldLocation(CZSealBody* seal, Vec3* position);

bool RespawnSeal(CZSealBody* seal);
bool RespawnLocalPlayer(void);

CZSealBody* SpawnAIBot(CCharacterType* character, const char* name, u32 teamMask, Matrix4x4 position, u32* outId);

CZSealBody* SpawnFriendlyAIBot(const char* name, u32* outId);

bool RemoveBot(u8 id);

#endif