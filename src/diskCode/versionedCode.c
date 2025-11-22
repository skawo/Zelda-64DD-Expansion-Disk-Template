#include "diskCode.h"
#include "../symbols1_0.h"
#include "../symbols1_1.h"
#include "../symbols1_2.h"

#define VERSIONED_TABLE_NAME(tablename, name) \
    static void* tablename##_Table[] = {(void*)name##_1_0, (void*)name##_1_1, (void*)0xFFFFFFFF, (void*)name##_1_2}

#define VERSIONED_TABLE(name) VERSIONED_TABLE_NAME(name, name)

#define VERSIONED_TABLE_VAL_NAME(tablename, name) \
    static u32 tablename##_Table[] = {name##_1_0, name##_1_1, 0xFFFFFFFF, name##_1_2}    

#define VERSIONED_TABLE_VAL(name) VERSIONED_TABLE_VAL_NAME(name, name)
  
#define VERSIONED_CALL(table, version, FuncType, ...) \
    ((FuncType)table[version])(__VA_ARGS__)

VERSIONED_TABLE(Audio_PlaySfxGeneral);
VERSIONED_TABLE(Actor_Spawn);
VERSIONED_TABLE(bcopy);
VERSIONED_TABLE(_codeSegmentStart);
VERSIONED_TABLE_NAME(funcTablePtr, D_800FEE70);
VERSIONED_TABLE(Font_LoadChar);
VERSIONED_TABLE(sFontWidths);
VERSIONED_TABLE(osEPiWriteIo);
VERSIONED_TABLE(osEPiReadIo);
VERSIONED_TABLE(osCartRomInit);
VERSIONED_TABLE(__locReadTimer);
VERSIONED_TABLE(__locSetTimer);
VERSIONED_TABLE(sprintf);
VERSIONED_TABLE_VAL_NAME(nesFont, _nes_font_staticSegmentRomStart);
VERSIONED_TABLE_VAL_NAME(engMsg, _nes_message_data_staticSegmentRomStart);


#define v_Audio_PlaySfxGeneral(gameVer, sfxId, pos, token, freqScale, vol, reverbAdd) \
    (((void (*)(u16, Vec3f*, u8, f32*, f32*, s8*)) \
        Audio_PlaySfxGeneral_Table[(gameVer)])(sfxId, pos, token, freqScale, vol, reverbAdd))

#define v_Actor_Spawn(gameVer, actorCtx, play, actorId, posX, posY, posZ, rotX, rotY, rotZ, params) \
    (((void (*)(ActorContext*, PlayState*, s16, f32, f32, f32, s16, s16, s16, s16)) \
        Actor_Spawn_Table[(gameVer)])(actorCtx, play, actorId, posX, posY, posZ, rotX, rotY, rotZ, params))

#define v_osEPiWriteIo(gameVer, handle, devAddr, data) \
    (((s32 (*)(OSPiHandle*, u32, u32)) \
        osEPiWriteIo_Table[(gameVer)])(handle, devAddr, data))

#define v_osEPiReadIo(gameVer, handle, devAddr, data) \
    (((s32 (*)(OSPiHandle*, u32, u32*)) \
        osEPiReadIo_Table[(gameVer)])(handle, devAddr, data))

#define v_osCartRomInit(gameVer) \
    (((OSPiHandle* (*)()) \
        osCartRomInit_Table[(gameVer)])())

#define v__locReadTimer(gameVer, time) \
    (((u8 (*)(__LOCTime*)) \
        __locReadTimer_Table[(gameVer)])(time))

#define v__locSetTimer(gameVer, time) \
    (((u8 (*)(__LOCTime*)) \
        __locSetTimer_Table[(gameVer)])(time))

#define v_sprintf(gameVer, dst, fmt, ...) \
    (((int (*)(char*, const char*, ...))sprintf_Table[gameVer])((dst), (fmt), __VA_ARGS__))

#define v_bcopy(gameVer, __src, __dest, __n) \
    (((void (*)(u8, const void*, void*, int)) \
        bcopy_Table[(gameVer)])(__src, __dest, __n))
