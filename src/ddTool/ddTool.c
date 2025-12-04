#include "ddTool.h"

typedef struct Yaz0Header
{
    char magic[4];
    u32 decSize;
    u32 compInfoOffset;
    u32 uncompDataOffset;
} Yaz0Header;

// This may return + one scanline (+0x280)
void* ddGetCurFrameBuffer()
{
    u32* viReg = (u32*)K0_TO_K1(VI_ORIGIN_REG);
    return (void*)K0_TO_K1(*viReg);     
}

u32 ddStrlen(const char* str) 
{
    const char* ptr = str;

    while (*ptr)
        ptr++;

    return ptr - str;
}

void ddMemcpy(void* dst, const void* src, int n)
{
    u8* p1 = (u8*)src;
    u8* p2 = (u8*)dst;

    for (int i = 0; i < n; i++)
    {
        *p2 = *p1;
        p1++;
        p2++;            
    }    
}

void* ddMemmove(void* dest, const void* src, int n)
{
    u8* d = (u8*)dest;
    u8* s = (u8*)src;

    if (d < s)
    {
        while (n--)
            *d++ = *s++;
    }
    else if (d > s)
    {
        d += n;
        s += n;

        while (n--)
            *--d = *--s;
    }

    return dest;
}

int ddMemcmp(const void* s1, const void* s2, int n)
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

void ddYaz0_Decompress(const u8* src, u8* dst, int compr_size)
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
        ddMemcpy(dst, src, compr_size);
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
*dst++ = backPtr[-1];
backPtr++;
                chunkSize--;
            }
            while (chunkSize != 0);
        }

        chunkHeader <<= 1;
        bitIdx--;

    }
    while (dst != dstEnd);
}

u16 ddGetSJisIndex(u8 c) 
{
    if (c >= '0' && c <= '9')
        return 146 + (c - '0');
    else if (c >= 'A' && c <= 'Z')
        return 156 + (c - 'A');
    else if (c >= 'a' && c <= 'z')
        return 182 + (c - 'a');
    else if (c == ' ')
        return 0;

    switch (c) 
    {
        case 33: return 9;    // !
        case 34: return 76;   // "
        case 35: return 83;   // #
        case 36: return 79;   // $
        case 37: return 82;   // %
        case 38: return 84;   // &
        case 39: return 75;   // '
        case 40: return 41;   // (
        case 41: return 42;   // )
        case 42: return 85;   // *
        case 43: return 59;   // +
        case 44: return 3;    // ,
        case 45: return 60;   // -     
        case 46: return 4;    // .
        case 47: return 30;   // /
        case 58: return 6;    // :
        case 59: return 7;    // ;
        case 60: return 49;   // <
        case 62: return 50;   // >
        case 63: return 8;    // ?
        case 91: return 45;   // [
        case 92: return 78;   // ¥
        case 93: return 31;   // (\)
        case 94: return 117;  // ^
        case 95: return 17;   // _
        case 96: return 37;   // `
        case 123: return 47;  // {
        case 124: return 35;  // |
        case 125: return 48;  // }
        case 126: return 32;  // ~
    }
    
    return 0xFFFF;
}

void ddDrawRGBA16ToFramebuffer(void* charTexBuf, s32 posX, s32 posY, u32 dx, s32 dy, s32 cy, void* frameBuf, s32 screenWidth)
{
    u16* src = (u16*)charTexBuf;
    u16* dst = (u16*)frameBuf;

    for (s32 y = 0; y < dy; y++) 
    {
        u16* srcRow = src + y * dx;

        for (s32 x = 0; x < dx; x++) 
        {
            u16 pix = srcRow[x];

            if (pix & 0x1)
                dst[posX + x + ((posY + (11 - cy) + y) * screenWidth)] = pix;
        }
    }
}