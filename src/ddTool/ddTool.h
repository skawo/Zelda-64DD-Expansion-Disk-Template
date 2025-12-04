#ifndef DDTOOL_H 
#define DDTOOL_H

#include "../common.h"
#include "../../include/ultra64.h"
#include "../../include/gfx.h"

#ifndef PREFIX
    #define PREFIX
#endif

#define ARRAY_COUNT(arr) (s32)(sizeof(arr) / sizeof(arr[0]))
#define ARRAY_COUNTU(arr) (u32)(sizeof(arr) / sizeof(arr[0]))

#define CAT2(a,b) a##b
#define CAT(a,b) CAT2(a,b)

#define FN(name) CAT(PREFIX, name)

void FN(ddMemcpy)(void* dst, const void* src, int n);
void FN(ddYaz0_Decompress)(const u8* src, u8* dst, int compr_size);
int FN(ddMemcmp)(const void* s1, const void* s2, int n);
void FN(ddMemfill)(void* dst, u8 byte, int n);
u16 FN(ddGetSJisIndex)(u8 c);
u32 FN(ddStrlen)(const char* str);
void* FN(ddGetCurFrameBuffer)();
void FN(ddClearFramebuffer)();
void* FN(ddMemmove)(void* dest, const void* src, int n);
void FN(ddDrawRGBA16ToFramebuffer)(void* charTexBuf, s32 posX, s32 posY, u32 dx, s32 dy, s32 cy, void* frameBuf, s32 screenWidth);

#define ddMemcpy FN(ddMemcpy)
#define ddMemcmp FN(ddMemcmp)
#define ddMemfill FN(ddMemfill)
#define ddYaz0_Decompress FN(ddYaz0_Decompress)
#define ddGetSJisIndex FN(ddGetSJisIndex)
#define ddStrlen FN(ddStrlen)
#define ddGetCurFrameBuffer FN(ddGetCurFrameBuffer)
#define ddMemmove FN(ddMemmove)
#define ddDrawRGBA16ToFramebuffer FN(ddDrawRGBA16ToFramebuffer)

// This macro exists so that PJ64 doesn't complain about being stuck in an infinite loop.
#define INFINITE_LOOP       \
    while (true)            \
    {                       \
        volatile u8 i;      \
        i++;                \
    }

#endif // DDTOOL_H