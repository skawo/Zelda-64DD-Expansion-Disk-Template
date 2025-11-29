#include "diskCache.h"
#include "../diskCode/diskCode.h"

#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

void ddCache_Init(DDCache* cache)
{
    is64Printf("Initing cache.\n");

    dd.vtable.osMallocInit(&cache->cacheArena, DDCACHE_START, (int)(DDCACHE_END - DDCACHE_START));
    
    if (cache->cacheArena.head)
        is64Printf("Free: %x\n", cache->cacheArena.head->size);
    
    for (int i = 0; i < DDCACHE_MAXFILES; i++)
    {
        cache->files[i].diskOffs = DDFILE_INVALID;
        cache->files[i].timeStamp = 0;
        cache->files[i].vram = NULL;
        cache->files[i].len = 0;
        cache->files[i].type = DDFILE_REGULAR;
        cache->files[i].sceneIDWhenLoaded = false;
    }
}

bool ddCache_AddFile(DDCache* cache, u32 fileDiskStart, void* addr, int len, u8 type)
{
    for (int i = 0; i < DDCACHE_MAXFILES; i++)
    {
        DDFile* checkedFile = &cache->files[i];

        if (checkedFile->diskOffs == DDFILE_INVALID)
        {
            checkedFile->diskOffs = fileDiskStart;
            checkedFile->len = len;
            checkedFile->timeStamp = dd.funcTablePtr->osGetTime();
            checkedFile->vram = addr;
            checkedFile->type = type;
            
            if (dd.play)
                checkedFile->sceneIDWhenLoaded = dd.play->sceneId;
            else
                checkedFile->sceneIDWhenLoaded = DDFILE_INVALIDSCENE;

            return true;
        }
    }    

    return false;
}

static void ddCache_InvalidateFile(DDCache* cache, DDFile* f)
{
    if (!f) 
        return;

    f->diskOffs = DDFILE_INVALID;
    f->timeStamp = 0;
    f->vram = NULL;
    f->len = 0;
}

static DDFile* ddCache_FindFile(DDCache* cache, void* ptr)
{
    for (int i = 0; i < DDCACHE_MAXFILES; ++i)
    {
        if (cache->files[i].diskOffs != DDFILE_INVALID && cache->files[i].vram == ptr)
            return &cache->files[i];
    }
    return NULL;
}

void ddCache_FreeFile(DDCache* cache, u32 fileDiskStart)
{
    DDFile* f = ddCache_FindFile(cache, (void*)fileDiskStart);
    dd.vtable.osFree(&cache->cacheArena, f->vram);
    ddCache_InvalidateFile(cache, f);
}

void ddCache_ClearAll(DDCache* cache)
{
    for (int i = 0; i < DDCACHE_MAXFILES; ++i)
    {
        DDFile* f = &cache->files[i];
        dd.vtable.osFree(&cache->cacheArena, f->vram);
        ddCache_InvalidateFile(cache, f);
    }    
}

bool ddCache_CanFileBeUnloaded(DDFile* file)
{
    if (file->diskOffs == DDFILE_INVALID)
        return false;
    else if (file->type == DDFILE_PERMANENT)
        return false;
    else if (dd.play && file->type == DDFILE_SCENE_PERMANENT && file->sceneIDWhenLoaded == dd.play->sceneId)
        return false; 

    return true;
}

void* ddCache_AllocFile(DDCache* cache, u32 fileDiskStart, int len, u8 type)
{
    int alignedLen = ALIGN16(len);

    while(true)
    {
        u32 outMaxFree = 0;
        u32 outFree = 0;
        u32 outAlloc = 0;
        dd.vtable.arenaImpl_GetSizes(&cache->cacheArena, &outMaxFree, &outFree, &outAlloc);
        u32 arenaSize = outFree + outAlloc;

        if ((u32)alignedLen > arenaSize) 
        {
            is64Printf("File too large: req=%x arena=%x\n", alignedLen, arenaSize);
            return NULL;
        }

        if ((u32)alignedLen <= outFree)
        {
            is64Printf("Allocing file %x, need %x, free=%x/%x\n", fileDiskStart, alignedLen, outFree, arenaSize);

            void* alloc = dd.vtable.osMalloc(&cache->cacheArena, alignedLen);

            if (alloc != NULL)
            {
                if (ddCache_AddFile(cache, fileDiskStart, alloc, alignedLen, type))
                {
                    is64Printf("Registered file %x @ %x\n", fileDiskStart, alloc);
                    return alloc;
                }
                else
                {
                    is64Printf("Ran out of file slots when allocating %x\n", fileDiskStart);
                    dd.vtable.osFree(&cache->cacheArena, alloc);
                    // Will evict file after this.
                }
            }
            else
            {
                is64Printf("osMalloc failed.\n");
            }
        }

        is64Printf("Not enough space for file %x, need %x, free=%x/%x\n", fileDiskStart, alignedLen, outFree, arenaSize);

        // Try to find a single existing cached file that is >= alignedLen (prefer oldest = smallest timestamp)
        DDFile* candidate = NULL;
        for (int i = 0; i < DDCACHE_MAXFILES; i++)
        {
            DDFile* checkedFile = &cache->files[i];

            if (checkedFile->diskOffs == DDFILE_INVALID) 
                continue;

            if ((u32)checkedFile->len >= (u32)alignedLen)
            {
                if (!ddCache_CanFileBeUnloaded(checkedFile))
                    continue;
                
                if (!candidate)
                    candidate = checkedFile;
                else if (checkedFile->timeStamp < candidate->timeStamp)
                    candidate = checkedFile;
            }
        }

        if (candidate)
        {
            is64Printf("Freeing file %x for file %x\n", candidate->diskOffs, fileDiskStart);
            dd.vtable.osFree(&cache->cacheArena, candidate->vram);
            ddCache_InvalidateFile(cache, candidate);
            // loop to try allocation again
            continue;
        }
        // No single file large enough; free oldest files until we have enough free space
        {
            u32 freeAfter = outFree;
            bool freed = false;

            while (freeAfter < (u32)alignedLen)
            {
                DDFile* oldestFile = NULL;

                for (int i = 0; i < DDCACHE_MAXFILES; i++)
                {
                    DDFile* checkedFile = &cache->files[i];

                    if (!ddCache_CanFileBeUnloaded(checkedFile))
                        continue;

                    if (!oldestFile)
                        oldestFile = checkedFile;
                    else if (checkedFile->timeStamp < oldestFile->timeStamp) 
                        oldestFile = checkedFile;
                }

                if (!oldestFile)
                    break;

                is64Printf("Freeing oldest file %x for file %x\n", oldestFile->diskOffs, fileDiskStart);

                void* startPtr = oldestFile->vram;
                u32 lenFreed = oldestFile->len;
                ddCache_InvalidateFile(cache, oldestFile);
                dd.vtable.osFree(&cache->cacheArena, startPtr);

                freeAfter += lenFreed;
                freed = true;
            }

            if (!freed && freeAfter < (u32)alignedLen)
            {
                is64Printf("Unable to allocate %x bytes\n", alignedLen);
                return NULL;
            }

            continue;
        }
    }
}

void* ddCache_LoadFile(DDCache* cache, u32 offset, u32 len, u8 type)
{
    for (int i = 0; i < DDCACHE_MAXFILES; i++)
    {
        DDFile* checkedFile = &cache->files[i];

        if (checkedFile->diskOffs == offset)
        {
            checkedFile->timeStamp = dd.funcTablePtr->osGetTime();
            is64Printf("Found file at %x\n", checkedFile->vram);

            return checkedFile->vram;
        }
    }

    void* alloc = ddCache_AllocFile(cache, offset, len, type);
    is64Printf("Allocated file at %x\n", alloc);

    if (alloc != NULL)
    {
        Disk_Load_MusicSafe(alloc, offset, len);
        return alloc;
    }
    else
    {
        is64Printf("Could not alloc file %x\n", offset);
        return NULL;
    }
}

void* ddCache_LoadFileTo(void* dest, DDCache* cache, u32 offset, u32 len)
{
    void* addr = ddCache_LoadFile(cache, offset, len, DDFILE_REGULAR);

    if (addr)
    {
        ddMemcpy(dest, addr, len);
        return dest;
    }

    return NULL;
}
