#include "diskCode.h"

#define VERSIONED_TABLE(name, ...) \
    static void* name[] = { __VA_ARGS__ }

#define VERSIONED_TABLE_VAL(name, ...) \
    static u32 name[] = { __VA_ARGS__ }    

#define VERSIONED_CALL(table, version, FuncType, ...) \
    ((FuncType)table[version])(__VA_ARGS__)
                                                /*  NTSC_1_0   */              /*   NTSC_1_1   */                                    /*   NTSC_1_2   */
VERSIONED_TABLE(PlaySfx_Table,                  (void*)0x800C806C,             (void*)0x800C823C,           (void*)0xFFFFFFFF,      (void*)0x800C88BC);
VERSIONED_TABLE(ActorSpawn_Table,               (void*)0x80025110,             (void*)0x80025110,           (void*)0xFFFFFFFF,      (void*)0x80025750);
VERSIONED_TABLE(bcopy_Table,                    (void*)0x80004DC0,             (void*)0x80004DC0,           (void*)0xFFFFFFFF,      (void*)0x80004FD0);
VERSIONED_TABLE(funcTablePtr_Table,             (void*)0x800FEE70,             (void*)0x800FF030,           (void*)0xFFFFFFFF,      (void*)0x800FF4B0);
VERSIONED_TABLE(mainCode_Table,                 (void*)0x800110A0,             (void*)0x800110A0,           (void*)0xFFFFFFFF,      (void*)0x800116E0);
VERSIONED_TABLE(Font_LoadChar_Table,            (void*)0x8005BCE4,             (void*)0x8005BCE4,           (void*)0xFFFFFFFF,      (void*)0x8005C374);
VERSIONED_TABLE(fontWidths_Table,               (void*)0x80112F40,             (void*)0x80113100,           (void*)0xFFFFFFFF,      (void*)0x801135F0);

VERSIONED_TABLE_VAL(nesFont_Table,              0x00928000,                    0x00928000,                  0xFFFFFFFF,              0x008ED000);
VERSIONED_TABLE_VAL(engMsg_Table,               0x0092D000,                    0x0092D000,                  0xFFFFFFFF,              0x0092D000);


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