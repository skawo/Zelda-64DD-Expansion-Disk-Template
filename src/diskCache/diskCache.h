#ifndef DDCACHE_H
#define DDCACHE_H

#include "../include/n64dd.h"
#include "../include/game.h"
#include "../include/libc64/os_malloc.h"
#include "../ddTool/ddTool.h"

#define DDCACHE_START (void*)0x80600000
#define DDCACHE_END (void*)0x80800000
#define DDCACHE_SIZE DDCACHE_END - DDCACHE_START
#define DDCACHE_MAXFILES 128
#define DDFILE_INVALID 0xFFFFFFFF
#define DDFILE_INVALIDSCENE 0xFFFF

#define DDFILE_REGULAR 0
#define DDFILE_PERMANENT 1
#define DDFILE_SCENE_PERMANENT 2

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
bool ddCache_AddFile(DDCache* cache, u32 fileDiskStart, void* addr, int len, u8 type);
void ddCache_FreeFile(DDCache* cache, u32 fileDiskStart);
void ddCache_ClearAll(DDCache* cache);
void* ddCache_AllocFile(DDCache* cache, u32 fileDiskStart, int len, u8 type);
void* ddCache_LoadFile(DDCache* cache, u32 offset, u32 len, u8 type);
void* ddCache_LoadFileTo(void* dest, DDCache* cache, u32 offset, u32 len);

#endif // DDCACHE_H