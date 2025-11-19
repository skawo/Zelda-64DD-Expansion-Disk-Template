#include "diskCode.h"

#define VERSIONED_TABLE(name, ...) \
    static void* name[] = { __VA_ARGS__ }

#define VERSIONED_CALL(table, version, FuncType, ...) \
    ((FuncType)table[version - 1])(__VA_ARGS__)
                                                /*  NTSC_1_0   */              /*   NTSC_1_1   */                                    /*   NTSC_1_2   */
VERSIONED_TABLE(PlaySfx_Table,                  (void*)0x800C806C,             (void*)0x800C823C,           (void*)0xFFFFFFFF,      (void*)0x800C88BC);
VERSIONED_TABLE(ActorSpawn_Table,               (void*)0x80025110,             (void*)0x80025110,           (void*)0xFFFFFFFF,      (void*)0x80025750);
VERSIONED_TABLE(bcopy_Table,                    (void*)0x80004DC0,             (void*)0x80004DC0,           (void*)0xFFFFFFFF,      (void*)0x80004FD0);
VERSIONED_TABLE(funcTablePtr_Table,             (void*)0x800FEE70,             (void*)0x800FF030,           (void*)0xFFFFFFFF,      (void*)0x800FF4B0);
VERSIONED_TABLE(mainCode_Table,                 (void*)0x800110A0,             (void*)0x800110A0,           (void*)0xFFFFFFFF,      (void*)0x800116E0);
VERSIONED_TABLE(engMsg_Table,                   (void*)0x92D000,               (void*)0x92D000,             (void*)0xFFFFFFFF,      (void*)0x92D000);

void Audio_PlaySfxGeneral_Versioned(u8 gameVer, u16 sfxId, Vec3f* pos, u8 token, f32* freqScale, f32* vol, s8* reverbAdd)
{
    typedef void (*PlaySfxFunc)(u16, Vec3f*, u8, f32*, f32*, s8*);
    VERSIONED_CALL(PlaySfx_Table, gameVer, PlaySfxFunc, sfxId, pos, token, freqScale, vol, reverbAdd);
}

void Actor_Spawn_Versioned(u8 gameVer, ActorContext* actorCtx, PlayState* play, s16 actorId, f32 posX, f32 posY, f32 posZ, s16 rotX, s16 rotY, s16 rotZ, s16 params)
{
    typedef void (*ActorSpawnFunc)(ActorContext*, PlayState*, s16, f32, f32, f32, s16, s16, s16, s16);
    VERSIONED_CALL(ActorSpawn_Table, gameVer, ActorSpawnFunc, actorCtx, play, actorId, posX, posY, posZ, rotX, rotY, rotZ, params);
}

void bcopy_Versioned(u8 gameVer, const void* __src, void* __dest, int __n)
{
    typedef void (*bcopyFunc)(const void*, void*, int);
    VERSIONED_CALL(bcopy_Table, gameVer, bcopyFunc, __src, __dest, __n);
}