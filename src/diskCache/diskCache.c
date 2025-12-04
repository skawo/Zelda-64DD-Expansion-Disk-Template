#include "diskCache.h"
#include "../diskCode/diskCode.h"

void ddCache_Init(DDCache* cache)
{
    is64Printf("Initing cache.\n");

    dd.vtable.osMallocInit(&cache->cacheArena, DDCACHE_START, (int)(DDCACHE_END - DDCACHE_START));
    
    if (cache->cacheArena.head)
        is64Printf("Free: %X\n", cache->cacheArena.head->size);
    
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

static bool ddCache_AddFile(DDCache* cache, u32 diskOffs, void* addr, int len, u8 type)
{
    for (int i = 0; i < DDCACHE_MAXFILES; i++)
    {
        DDFile* checkedFile = &cache->files[i];

        if (checkedFile->diskOffs == DDFILE_INVALID)
        {
            checkedFile->diskOffs = diskOffs;
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

void ddCache_FreeFile(DDCache* cache, u32 diskOffs)
{
    DDFile* f = ddCache_FindFile(cache, (void*)diskOffs);

    if (!f)
        return;

    dd.vtable.osFree(&cache->cacheArena, f->vram);
    ddCache_InvalidateFile(cache, f);
}

void ddCache_FreeAll(DDCache* cache)
{
    for (int i = 0; i < DDCACHE_MAXFILES; i++)
    {
        DDFile* f = &cache->files[i];

        if (f->diskOffs != DDFILE_INVALID)
        {
            dd.vtable.osFree(&cache->cacheArena, f->vram);
            ddCache_InvalidateFile(cache, f);
        }
    }    
}

static void ddCache_PrintoutNodes(DDCache* cache)
{
    #ifdef DEBUGTOOLS
    is64Printf("\n======CACHE STATE======\n");

    ArenaNode* curNode = cache->cacheArena.head;

    while (curNode)
    {
        is64Printf("Node: %X, Size: %X, Status: %s\n", curNode, curNode->size, curNode->isFree ? "Free" : "Not Free");
        curNode = curNode->next;
    }

    is64Printf("==========================\n");  
    #endif 
}

#define NODE_MAGIC 0x7373

void ddCache_Defragment(DDCache* cache)
{
    ddCache_PrintoutNodes(cache);

    is64Printf("\nDefragmenting...\n");

    Arena* arena = &cache->cacheArena;

    ArenaNode* node = arena->head;
    ArenaNode* newHead = NULL;
    ArenaNode* prevNode = NULL;

    u8* writePtr = (u8*)arena->start;

    // Make all the blocks contiguous
    while (node) 
    {
        ArenaNode* next = node->next;

        if (!node->isFree) 
        {
            u32 totalSize = sizeof(ArenaNode) + node->size;
            void* nodeAlloc = NodeData(node);

            // If the next free block is now where it should logically be if the files were contiguous...
            if ((u8*)node != writePtr)
            {
                // Relocate DD file
                for (int i = 0; i < DDCACHE_MAXFILES; i++)
                {
                    DDFile* f = &cache->files[i];

                    if (f->vram == nodeAlloc)
                    {
                        void* newAlloc = NodeData(writePtr);
                        is64Printf("Re-registered file %X to %X\n", f->diskOffs, newAlloc);
                        f->vram = newAlloc;
                        break;
                    }
                }

                is64Printf("Moving node %X to %X\n", node, writePtr);
                ddMemmove(writePtr, node, totalSize);
            }

            // Relocate the header
            node = (ArenaNode*)writePtr;
            node->prev = prevNode;
            node->next = NULL;

            if (prevNode)
                prevNode->next = node;

            if (!newHead)
                newHead = node;

            prevNode = node;
            writePtr += totalSize;
        }

        node = next;
    }

    // Create new empty header at the end
    ArenaNode* freeNode = NULL;

    u8* endOfArena = (u8*)arena->start;
    endOfArena += arena->size;

    u32 remaining = endOfArena - writePtr;

    if (remaining > sizeof(ArenaNode)) 
    {
        freeNode = (ArenaNode*)writePtr;

        freeNode->magic = NODE_MAGIC;
        freeNode->isFree = true;
        freeNode->size = remaining - sizeof(ArenaNode);
        freeNode->prev = prevNode;
        freeNode->next = NULL;

        if (prevNode)
            prevNode->next = freeNode;
    }

    arena->head = newHead ? newHead : freeNode;

    if (arena->head)
        arena->head->prev = NULL;

    ddCache_PrintoutNodes(cache);
}

static bool ddCache_CanFileBeUnloaded(DDFile* file)
{
    if (!file ||
        (file->diskOffs == DDFILE_INVALID) ||
        (file->type == DDFILE_PERMANENT) ||
        (dd.play && file->type == DDFILE_SCENE_PERMANENT && file->sceneIDWhenLoaded == dd.play->sceneId))
        return false;

    return true;
}

static void* ddCache_AllocFile(DDCache* cache, u32 diskOffs, int len, u8 type)
{
    int alignedLen = ALIGN16(len);

    while (true)
    {
        u32 outMaxFree, outFree, outAlloc;
        u32 arenaSize = cache->cacheArena.size;
        dd.vtable.arenaImpl_GetSizes(&cache->cacheArena, &outMaxFree, &outFree, &outAlloc);

        bool forceFreeFileSlot = false;

        if ((u32)alignedLen > arenaSize) 
        {
            is64Printf("File too large: req=%X arena=%X\n", alignedLen, arenaSize);
            return NULL;
        }

        if ((u32)alignedLen <= outFree)
        {
            is64Printf("Allocing file %X, need %X, free=%X/%X\n", diskOffs, alignedLen, outFree, arenaSize);

            void* alloc = dd.vtable.osMalloc(&cache->cacheArena, alignedLen);

            if (alloc != NULL)
            {
                if (ddCache_AddFile(cache, diskOffs, alloc, alignedLen, type))
                {
                    is64Printf("Registered file %X @ %X\n", diskOffs, alloc);
                    return alloc;
                }
                else
                {
                    is64Printf("Ran out of file slots when allocating %X, force-freeing a file.\n", diskOffs);
                    dd.vtable.osFree(&cache->cacheArena, alloc);
                    forceFreeFileSlot = true;
                    // Will evict file after this.
                }
            }
            else
            {
                is64Printf("osMalloc failed due to fragmentation, force-freeing a file.\n");
                forceFreeFileSlot = true;
            }
        }
        else
            is64Printf("Not enough space for file %X, need %X, free=%X/%X\n", diskOffs, alignedLen, outFree, arenaSize);

        // Try to find the oldest single existing cached file that is >= alignedLen
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
            is64Printf("Freeing file %X for file %X\n", candidate->diskOffs, diskOffs);
            dd.vtable.osFree(&cache->cacheArena, candidate->vram);
            ddCache_InvalidateFile(cache, candidate);
            // loop to try allocation again
            continue;
        }
        // No single file large enough; free oldest files until we have enough free space
        else
        {
            u32 freeAfter = outFree;
            bool freed = false;

            while (freeAfter < (u32)alignedLen || forceFreeFileSlot)
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

                is64Printf("Freeing oldest file %X for file %X\n", oldestFile->diskOffs, diskOffs);

                void* startPtr = oldestFile->vram;
                u32 lenFreed = oldestFile->len;
                ddCache_InvalidateFile(cache, oldestFile);
                dd.vtable.osFree(&cache->cacheArena, startPtr);

                freeAfter += lenFreed;
                freed = true;
                forceFreeFileSlot = false;
            }

            if (!freed && freeAfter < (u32)alignedLen)
            {
                is64Printf("Unable to allocate %X bytes\n", alignedLen);
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
            checkedFile->sceneIDWhenLoaded = dd.play->sceneId; 
            is64Printf("Found file at %X\n", checkedFile->vram);

            return checkedFile->vram;
        }
    }

    void* alloc = ddCache_AllocFile(cache, offset, len, type);
    is64Printf("Allocated file at %X\n", alloc);

    if (alloc != NULL)
    {
        Disk_Load(alloc, offset, len);
        return alloc;
    }
    else
    {
        is64Printf("Could not alloc file %X\n", offset);
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
