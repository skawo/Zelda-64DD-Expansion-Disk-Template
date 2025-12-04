#ifndef DDCACHE_H
#define DDCACHE_H

#include "../common.h"
#include "../include/n64dd.h"
#include "../include/game.h"
#include "../include/libc64/os_malloc.h"
#include "../ddTool/ddTool.h"

#define DDCACHE_START (void*)0x80600000

#ifdef SAVESTATES
    // PJ64 compatibility...
    #define DDCACHE_END (void*)0x807EFFF0
#else
    #define DDCACHE_END (void*)0x80800000
#endif

#define DDCACHE_SIZE DDCACHE_END - DDCACHE_START
#define DDCACHE_MAXFILES 128
#define DDFILE_INVALID 0xFFFFFFFF
#define DDFILE_INVALIDSCENE 0xFFFF

#define DDFILE_REGULAR 0
#define DDFILE_PERMANENT 1
#define DDFILE_SCENE_PERMANENT 2

#define NodeData(n) ((void*)((u8*)(n) + sizeof(ArenaNode)))

typedef struct DDFile
{
    OSTime timeStamp;
    u32 diskOffs;
    u32 len;
    void* vram;
    u16 sceneIDWhenLoaded;
    u8 type;
} DDFile;

typedef struct DDCache
{
    DDFile files[DDCACHE_MAXFILES];
    Arena cacheArena;
} DDCache;

void ddCache_Init(DDCache* cache);
static bool ddCache_AddFile(DDCache* cache, u32 diskOffs, void* addr, int len, u8 type);
void ddCache_FreeFile(DDCache* cache, u32 diskOffs);
void ddCache_FreeAll(DDCache* cache);
static void* ddCache_AllocFile(DDCache* cache, u32 diskOffs, int len, u8 type);
void* ddCache_LoadFile(DDCache* cache, u32 offset, u32 len, u8 type);
void* ddCache_LoadFileTo(void* dest, DDCache* cache, u32 offset, u32 len);
void ddCache_Defragment(DDCache* cache);

#endif // DDCACHE_H