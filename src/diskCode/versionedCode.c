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
VERSIONED_TABLE(_Font_LoadChar_Table,           (void*)0x8005BCE4,             (void*)0x8005BCE4,           (void*)0xFFFFFFFF,      (void*)0x8005C374);
VERSIONED_TABLE(fontWidths_Table,               (void*)0x80112F40,             (void*)0x80113100,           (void*)0xFFFFFFFF,      (void*)0x801135F0);
VERSIONED_TABLE(osEPiWriteIo_Table,             (void*)0x80005800,             (void*)0x80005800,           (void*)0xFFFFFFFF,      (void*)0x80005B70);
VERSIONED_TABLE(osCartRomInit_Table,            (void*)0x80005680,             (void*)0x80005680,           (void*)0xFFFFFFFF,      (void*)0x800059F0);
VERSIONED_TABLE(osEPiReadIo_Table,              (void*)0x80005630,             (void*)0x80005630,           (void*)0xFFFFFFFF,      (void*)0x80005840);
VERSIONED_TABLE(locReadTimer_Table,             (void*)0x801CEE94,             (void*)0x801CF034,           (void*)0xFFFFFFFF,      (void*)0x801CF714);
VERSIONED_TABLE(locSetTimer_Table,              (void*)0x801CF004,             (void*)0x801CF1A4,           (void*)0xFFFFFFFF,      (void*)0x801CF884);
VERSIONED_TABLE(sprintf_Table,                  (void*)0x800CE7B4,             (void*)0x800CE974,           (void*)0xFFFFFFFF,      (void*)0x800CEFF4);

VERSIONED_TABLE_VAL(nesFont_Table,              0x00928000,                    0x00928000,                  0xFFFFFFFF,              0x008ED000);
VERSIONED_TABLE_VAL(engMsg_Table,               0x0092D000,                    0x0092D000,                  0xFFFFFFFF,              0x0092D000);


#define Audio_PlaySfxGeneral_Versioned(gameVer, sfxId, pos, token, freqScale, vol, reverbAdd) \
    (((void (*)(u16, Vec3f*, u8, f32*, f32*, s8*)) \
        PlaySfx_Table[(gameVer)])(sfxId, pos, token, freqScale, vol, reverbAdd))

#define Actor_Spawn_Versioned(gameVer, actorCtx, play, actorId, posX, posY, posZ, rotX, rotY, rotZ, params) \
    (((void (*)(ActorContext*, PlayState*, s16, f32, f32, f32, s16, s16, s16, s16)) \
        ActorSpawn_Table[(gameVer)])(actorCtx, play, actorId, posX, posY, posZ, rotX, rotY, rotZ, params))

#define osEPiWriteIo_Versioned(gameVer, handle, devAddr, data) \
    (((s32 (*)(OSPiHandle*, u32, u32)) \
        osEPiWriteIo_Table[(gameVer)])(handle, devAddr, data))

#define osEPiReadIo_Versioned(gameVer, handle, devAddr, data) \
    (((s32 (*)(OSPiHandle*, u32, u32*)) \
        osEPiReadIo_Table[(gameVer)])(handle, devAddr, data))

#define osCartRomInit_Versioned(gameVer) \
    (((OSPiHandle* (*)()) \
        osCartRomInit_Table[(gameVer)])())

#define __locReadTimer_Versioned(gameVer, time) \
    (((u8 (*)(__LOCTime*)) \
        locReadTimer_Table[(gameVer)])(time))

#define __locSetTimer_Versioned(gameVer, time) \
    (((u8 (*)(__LOCTime*)) \
        locSetTimer_Table[(gameVer)])(time))

#define sprintf_Versioned(gameVer, dst, fmt, ...) \
    (((int (*)(char*, const char*, ...))sprintf_Table[gameVer])((dst), (fmt), __VA_ARGS__))

#define bcopy_Versioned(gameVer, __src, __dest, __n) \
    (((void (*)(u8, const void*, void*, int)) \
        bcopy_Table[(gameVer)])(__src, __dest, __n))
