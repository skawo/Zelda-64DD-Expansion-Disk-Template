#include "ddTool.h"

typedef struct Yaz0Header
{
    char magic[4];
    u32 decSize;
    u32 compInfoOffset;
    u32 uncompDataOffset;
} Yaz0Header;

void ddMemcpy(u8* src, u8* dst, int n)
{
    for (int i = 0; i < n; i++)
    {
        *dst = *src;
        dst++;
        src++;            
    }    
}

int ddMemcmp(void* s1, void* s2, int n)
{
    u8* p1 = (u8*)s1;
    u8* p2 = (u8*)s2;

    for (int i = 0; i < n; i++) 
    {
        if (p1[i] != p2[i])
            return (p1[i] < p2[i]) ? -1 : 1;
    }

    return 0;
}

void ddMemfill(void* dst, u8 byte, int n)
{
    u8* p = (u8*)dst;
    
    for (int i = 0; i < n; i++)
        p[i] = byte;
}

void ddYaz0_Decompress(u8* src, u8* dst, int compr_size)
{
    Yaz0Header* header = (Yaz0Header*)src;
    u32 bitIdx = 0;
    u8* dstEnd = dst + header->decSize;
    u32 chunkHeader;
    u32 nibble;
    u8* backPtr;
    u32 chunkSize;
    u32 off;

    // File is not compressed...
    if (*(u32*)src != 0x59617A30)
    {
        ddMemcpy(src, dst, compr_size);
        return;
    }

    src += sizeof(Yaz0Header);

    do
    {
        if (bitIdx == 0)
        {
            chunkHeader = *src++;
            bitIdx = 8;
        }

        if (chunkHeader & (1 << 7))
        {
            *dst = *src;
            dst++;
            src++;
        }
        else
        {
            off = ((*src & 0xF) << 8 | *(src + 1));
            nibble = *src >> 4;
            backPtr = dst - off;
            src += 2;

            chunkSize = (nibble == 0)
                            ? (u32)(*src++ + 0x12)
                            : nibble + 2;

            do
            {
                *dst++ = *(backPtr++ - 1);
                chunkSize--;
            }
            while (chunkSize != 0);
        }

        chunkHeader <<= 1;
        bitIdx--;

    }
    while (dst != dstEnd);
}
