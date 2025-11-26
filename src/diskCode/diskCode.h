#ifndef DISKC_H 
#define DISKC_H

#include "../../include/n64dd.h"
#include "../../include/save.h"
#include "../../include/message.h"
#include "../../include/gfx.h"
#include "../../include/game.h"
#include "../../include/play_state.h"
#include "../../include/player.h"
#include "../../include/controller.h"
#include "../../include/sfx.h"
#include "../../include/fault.h"
#include "../../include/audio.h"
#include "../../include/map_select_state.h"

#include "../ddTool/ddTool.h"

#include "../filesystem/vtables.h"
#include "../filesystem/filesystem.h"

//#define DVDLOGO
#define SIGN_CLOCK
#define SAVESTATES          // DISK_TYPE must be 4 or below
//#define DDIPL_FONT
#define MAP_SELECT
#define DEBUGTOOLS
#define ARWING

typedef struct ddFuncPointers ddFuncPointers;
typedef struct ddHookTable ddHookTable;
struct ddHookTable hookTable;

typedef void (*DiskInitFunc)(ddFuncPointers*, ddHookTable*);   

typedef struct diskInfo 
{
    /* 0x000 */ void* diskStart;     
    /* 0x004 */ void* diskEnd;       
    /* 0x008 */ void* vramStart; 
    /* 0x00C */ void* vramEnd; 
    /* 0x010 */ ddHookTable* hookTablePtr;
    /* 0x014 */ char unk_014[0x104];
} diskInfo; // size = 0x118

typedef struct ddFuncPointers 
{
    void (*loadFromDisk)(void* dest, s32 offset, s32 size);
    void* unk_null_1;
    struct RegEditor* regEditor;
    void (*faultRemoveClient)(FaultClient* client);
    void (*faultAddClient)(FaultClient* client, void* callback, void* arg0, void* arg1);
    void (*faultDrawText)(s32 x, s32 y, const char* fmt, ...);
    void (*faultWaitForInput)();
    void (*faultAddHungupAndCrashImpl)(const char* exp1, const char* exp2);
    void (*faultAddHungupAndCrash)(const char* file, int line);
    void (*stubFunc_800AD598)(s32 arg0, s32 arg1, s32 arg2);
    void (*printf)(PrintCallback pfn, void* arg, const char* fmt, va_list ap);
    void (*osCreateThread)(OSThread* thread, OSId id, void (*entry)(void*), void* arg, void* sp, OSPri pri);
    void (*osDestroyThread)(OSThread* thread);
    void (*osYieldThread)();
    void (*osStartThread)(OSThread* thread);
    void (*osStopThread)(OSThread* thread);
    OSId (*osGetThreadId)(OSThread* thread);
    void (*osSetThreadPri)(OSThread* thread, OSPri pri);
    OSPri (*osGetThreadPri)(OSThread* thread);
    void (*osCreateMesgQueue)(OSMesgQueue* mq, OSMesg* msg, s32 count);
    s32 (*osSendMesg)(OSMesgQueue* mq, OSMesg msg, s32 flag);
    s32 (*osJamMesg)(OSMesgQueue* mq, OSMesg msg, s32 flag);
    s32 (*osRecvMesg)(OSMesgQueue* mq, OSMesg* msg, s32 flag);
    void (*osSetEventMesg)(OSEvent e, OSMesgQueue* mq, OSMesg msg);
    OSIntMask (*osGetIntMask)();
    void (*osSetIntMask)(OSIntMask);
    void (*osInvalDCache)(void* vaddr, s32 nbytes);
    void (*osInvalICache)(void* vaddr, s32 nbytes);
    void (*osWritebackDCache)(void* vaddr, s32 nbytes);
    void (*osWritebackDCacheAll)();
    OSTime (*osGetTime)();
    void (*osSetTime)(OSTime time);
    s32 (*osSetTimer)(OSTimer* timer, OSTime countdown, OSTime interval, OSMesgQueue* mq, OSMesg msg);
    s32 (*osStopTimer)(OSTimer* timer);
    struct SaveContext* saveContext;
    s32 (*dmaMgrRequestAsync)(DmaRequest* req, void* ram, uintptr_t vrom, size_t size, u32 unk5, OSMesgQueue* queue, OSMesg msg);
    s32 (*dmaMgrRequestSync)(void* ram, uintptr_t vrom, size_t size);
    void (*dmaFromDriveRom)(void* ram, uintptr_t rom, size_t size);
    void (*cutsceneHandleEntranceTriggers)(struct PlayState* play);
    uintptr_t* gSegments;
    s32 (*flags_GetEventChkInf)(s32 flag);
    void (*flags_SetEventChkInf)(s32 flag);
    void* unk_null_2;
    void* unk_null_3;
} ddFuncPointers; // size = 0xB0

typedef struct ddHookTable 
{
    /* 0x000 */ void (*diskInit)(ddFuncPointers*, ddHookTable*);     
    /* 0x004 */ void (*diskDestroy)(void);
    /* 0x008 */ void (*loadRoom)(struct PlayState* play, struct RoomContext* roomCtx, s32 roomNum);
    /* 0x00C */ void (*sceneInit)(struct PlayState* play);
    /* 0x010 */ void (*playInit)(struct PlayState* play);
    /* 0x014 */ void (*playDestroy)(struct PlayState* play);
    /* 0x018 */ s32 (*mapDataInit)(struct MapData**);
    /* 0x01C */ s32 (*mapDataDestroy)(struct MapData**);
    /* 0x020 */ s32 (*mapDataSetDungeons)(struct MapData*);
    /* 0x024 */ s32 (*mapExpDestroy)(void);
    /* 0x028 */ s32 (*mapExpTextureLoadDungeons)(struct PlayState*);
    /* 0x02C */ s32 (*mapMarkInit)(MapMarkData***);
    /* 0x030 */ s32 (*mapMarkDestroy)(MapMarkData***);
    /* 0x034 */ void (*pauseMapMarkInit)(PauseMapMarksData**);
    /* 0x038 */ void (*pauseMapMarkDestroy)(PauseMapMarksData**);
    /* 0x03C */ void (*kaleidoInit)(void);
    /* 0x040 */ void (*kaleidoDestroy)(void);
    /* 0x044 */ s32 (*kaleidoLoadDungeonMap)(struct PlayState*);
    /* 0x048 */ struct SceneTableEntry* (*getSceneEntry)(s32 sceneId, struct SceneTableEntry* sceneTable);
    /* 0x04C */ char unk_4C[0x08];
    /* 0x054 */ s32 (*handleEntranceTriggers)(struct PlayState*);
#if OOT_PAL
    /* 0x058 */ void (*setMessageTables)(struct MessageTableEntry**, struct MessageTableEntry**, struct MessageTableEntry**, struct MessageTableEntry**);
#else
    /* 0x058 */ void (*setMessageTables)(struct MessageTableEntry**, struct MessageTableEntry**, struct MessageTableEntry**);
#endif
    /* 0x05C */ char unk_5C[0x4];
    /* 0x060 */ s32 (*loadCreditsMsg)(struct Font*);
#if OOT_PAL
    /* 0x064 */ s32 (*loadEnglishMsg)(struct Font*);
    /* 0x068 */ s32 (*loadGermanMsg)(struct Font*);
    /* 0x06C */ s32 (*loadFrenchMsg)(struct Font*);
#else
    /* 0x064 */ s32 (*loadJapaneseMsg)(struct Font*);
    /* 0x068 */ s32 (*loadEnglishMsg)(struct Font*);
#endif
    /* 0x06C */ void (*sceneDraw)(struct PlayState*, SceneDrawConfigFunc*);
    /* 0x070 */ s32 (*asyncDma)(struct DmaRequest* req, void* ram, uintptr_t vrom, size_t size, u32 unk, OSMesgQueue* queue, OSMesg msg);
    /* 0x074 */ void (*gameStateUpdate)(struct GameState*);
    /* 0x078 */ s32 (*cutsceneSetScript)(struct PlayState*, void*, void*);
} ddHookTable; // size = ?

#ifdef SAVESTATES
    typedef struct DDSavedState
    {
        char magic[16];
        int destinationScene;
        int musicId;
        int stateLoadCounter;
        int linkAge;
        int destinationEntrance;
    } DDSavedState;

    #define STATE_MAGIC    "N64DD_SAVE_STATE"
    #define CHECKING_MSG   "Checking saved state..."
    #define NO_SAVE_MSG    "No saved state found."
    #define SAVING_MSG     "Saving state to disk."
    #define LOADING_MSG    "Loading state from disk."
    #define PLEASE_WAIT    "Please wait..."
#endif

typedef struct DDState
{
    PlayState* play;
    ddFuncPointers* funcTablePtr;
    ddHookTable* hookTablePtr;
    s8 gameVersion;
    OSPiHandle* sISVHandle;
    VersionVTable vtable;

    #ifdef SAVESTATES
        DDSavedState sState;
    #endif
} DDState;

DDState dd;
u8 msgGfxBuf[0x80] __attribute__((__aligned__(64)));

#ifdef DEBUGTOOLS
    #define gISVDbgPrnAdrs ((ISVDbg*)0xB3FF0000)
    #define ASCII_TO_U32(a, b, c, d) ((u32)((a << 24) | (b << 16) | (c << 8) | (d << 0)))
#endif

extern VersionVTable* VTABLE_1_0;
extern VersionVTable* VTABLE_1_1;
extern VersionVTable* VTABLE_1_2;

void* vTableDiskAddrs[] = {&VTABLE_1_0, &VTABLE_1_1, NULL, &VTABLE_1_2};

void Disk_Init(ddFuncPointers* funcTablePtr, ddHookTable* hookTablePtr);
void Disk_Destroy();
void Disk_PlayInit(struct PlayState* play);
void Disk_PlayDestroy(struct PlayState* play);
void Disk_SceneInit(struct PlayState* play);
void Disk_SceneDraw(struct PlayState* play, SceneDrawConfigFunc* func);
void Disk_GameState(struct GameState* state);
void Disk_KaleidoDestroy();
s32 Disk_GetENGMessage(struct Font*);
void Disk_SetMessageTables(struct MessageTableEntry** Japanese, struct MessageTableEntry** English, struct MessageTableEntry** Credits);
struct SceneTableEntry* Disk_GetSceneEntry(s32 sceneId, struct SceneTableEntry* sceneTable);
void Disk_LoadRoom(struct PlayState* play, struct RoomContext* roomCtx, s32 roomNum);

void DrawRect(Gfx** gfxp, u8 r, u8 g, u8 b, u32 PosX, u32 PosY, u32 Sizex, u32 SizeY);
void ShowFullScreenGraphic(void* graphic, u32 graphicLen, bool halt);
void Draw64DDDVDLogo(struct PlayState* play);
void SpawnArwing(struct PlayState* play);
void _isPrintfInit();
void* _is_proutSyncPrintf(void* arg, const char* str, unsigned int count);
void is64Printf(const char* fmt, ...);
void DoClockDisplayOnLinkHouseSign(struct PlayState* play);
void RestoreMapSelect(struct PlayState* play);
void DoSaveStates(struct PlayState* play);
void Disk_Read_MusicSafe(void* dest, s32 offset, s32 size);
void Disk_Write(void* data, u32 diskAddr, u32 len);
void Disk_Write_MusicSafe(void* data, u32 diskAddr, u32 len);
void PrintTextLineToFb(u8* frameBuffer, char* msg, int xPos, int yPos, bool fontStyle);

extern void* __Disk_Init_K1;
extern void* __Disk_Start;
extern void* __Disk_End;
extern void* __Disk_VramStart;
extern void* __Disk_VramEnd;
extern void* __entry;
extern void* RAM_LENGTH;
extern void* ROM_LENGTH;

#define SEGMENT_STATIC_END 0x80800000
#define SEGMENT_STATIC_START 0x80700000
#define SEGMENT_STATIC_START2 0x80780000
#define ROOMS_START 0x80600000
#define RAM_START (u32)&__entry
#define RAM_LENGTH (u32)&RAM_LENGTH
#define ROM_LENGTH (u32)&ROM_LENGTH

#define SAVE_ID "64DD"

#endif // DISKC_H