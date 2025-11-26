#ifndef VTABLES_H
#define VTABLES_H

#include "../include/n64dd.h"
#include "../include/save.h"
#include "../include/message.h"
#include "../include/gfx.h"
#include "../include/game.h"
#include "../include/play_state.h"
#include "../include/player.h"
#include "../include/controller.h"
#include "../include/sfx.h"
#include "../include/fault.h"

typedef struct
{
    void (*audioPlaySfxGeneral)(u16, Vec3f*, u8, f32*, f32*, s8*);
    void (*actorSpawn)(ActorContext*, PlayState*, s16, f32, f32, f32, s16, s16, s16, s16);
    void (*bcopy)(u8, const void*, void*, int);
    void* CodeSegmentStart;
    void* funcTablePtr;
    void (*fontLoadChar)(u16);
    f32* sFontWidths;
    s32 (*osEPiWriteIo)(OSPiHandle*, u32, u32);
    s32 (*osEPiReadIo)(OSPiHandle*, u32, u32*);
    OSPiHandle* (*osCartRomInit)();
    u8 (*locReadTimer)(__LOCTime*);
    u8 (*locSetTimer)(__LOCTime*);
    int (*sprintf)(char*, const char*, ...);
    u32 ENGLISH_MESSAGE_DATA;
    u32 ENGLISH_FONT;
    Vec3f* gSfxDefaultPos;
    f32* gSfxDefaultFreqAndVolScale;
    s8* gSfxDefaultReverb;
    void (*titleCard_initPlaceName) (PlayState* play, TitleCardContext* titleCtx, void* texture, s32 x, s32 y, s32 width, s32 height, s32 delay);
    void (*mapSelectInit) (GameState* thisx);
    Gfx* (*gfxOpen) (Gfx* gfx);
    Gfx* (*gfxClose) (Gfx* gfx, Gfx* dst);
    void* (*gfxAlloc) (Gfx** gfxP, u32 size);
    vu8* haltMusicForDiskDMA;
    void* (*diskWrite) (void* arg0, s32 arg1, s32 arg2);
    void* (*diskRead) (s32 arg0, void* arg1, s32 arg2);
    void* (*byteToLBAandOffset) (s32 arg0, s32* arg1, s32* arg2);
    s32 (*getLBALength) (s32 startLBA);
    void** diskBuffer;
    void (*markDDUnavailable) ();
    void (*stopMusicThread) ();
    void (*markDDAvailable) ();
    void (*restartMusicThread) ();
    u32* audioCtx;
    void (*audio_StopBgmAndFanfare) (u16 fadeOutDuration);
    void (*audio_QueueSeqCmd) (u32 cmd);
    void* (*clearFrameBuffer) (void* arg0);
    void (*sleep) (u32 msec);
    void* (*printCharToFramebuffer) (void* charTexBuf, s32 posX, s32 posY, s32 dx, s32 dy, s32 cy, void* frameBuf, s32 screenWidth);
} VersionVTable;

#endif // VTABLES_H