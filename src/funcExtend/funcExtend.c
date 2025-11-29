#include "funcExtend.h"

// These perform their respective action without killing the audio engine.
void Disk_Load_MusicSafe(void* dest, s32 offset, s32 size)
{
    *dd.vtable.haltMusicForDiskDMA = 1;
    dd.funcTablePtr->loadFromDisk(dest, offset, size); 
}

void Disk_Write_MusicSafe(void* data, u32 diskAddr, u32 len)
{
    *dd.vtable.haltMusicForDiskDMA = 1;
    Disk_Write(data, diskAddr, len);
}

// Write to disk at specified byte address.
// The actual location on the disk will be + 0x785C8, because the system area is not counted, and LBA 1 is also not counted.
// DISK_TYPE governs where data can be written to the disk.
void Disk_Write(void* data, u32 diskAddr, u32 len)
{
    s32 lba_start, offset_start;
    s32 lba_end, offset_end;

    dd.vtable.markDDUnavailable();
    dd.vtable.stopMusicThread();

    dd.vtable.byteToLBAandOffset(diskAddr, &lba_start, &offset_start);
    dd.vtable.byteToLBAandOffset(diskAddr + len, &lba_end, &offset_end);

    *dd.vtable.ddStartOpTime = 0;

    // If the write is perfectly aligned to a LBA, then just write the data as-is.
    if (offset_start == 0)
    {
        dd.vtable.diskWrite((void*)lba_start, (s32)data, len);

        // Wait for completion
        while (dd.vtable.ddUnkFunc6() != 0)
            dd.vtable.sleepUsec(16666);
        while (dd.vtable.ddUnkFunc7() != 0)
            dd.vtable.sysFreeze();
    }
    else
    {
        s32 startLbaLen = dd.vtable.getLBALength(lba_start);
        s32 startLbaWrite = startLbaLen - offset_start;

        // If the write is not aligned, then read the whole LBA to buffer...
        dd.vtable.diskLoad((s32)lba_start,
                           (void*)*dd.vtable.diskBuffer,
                           startLbaLen);

        // Write new data...
        ddMemcpy((void*)((u32)*dd.vtable.diskBuffer + offset_start),
                 (void*)((u32)data),
                 startLbaWrite);

        // Then write the whole LBA.
        dd.vtable.diskWrite((void*)lba_start,
                            (s32)*dd.vtable.diskBuffer,
                            startLbaLen);

        // Wait for completion of the first block
        while (dd.vtable.ddUnkFunc6() != 0)
            dd.vtable.sleepUsec(16666);
        while (dd.vtable.ddUnkFunc7() != 0)
            dd.vtable.sysFreeze();

        // If there's more LBAs, write them.
        if (lba_end != lba_start)
        {
            dd.vtable.diskWrite((void*)(lba_start + 1),
                                (s32)data + startLbaWrite,
                                len - startLbaWrite);

            while (dd.vtable.ddUnkFunc6() != 0)
                dd.vtable.sleepUsec(16666);
            while (dd.vtable.ddUnkFunc7() != 0)
                dd.vtable.sysFreeze();
        }
    }

    // Finalize, idk
    if (*dd.vtable.ddStartOpTime != 0) 
    {
        s32 elapsed =
            (dd.funcTablePtr->osGetTime() - *dd.vtable.ddStartOpTime) * 64 / 3000;

        if (1000000 - elapsed > 0)
            dd.vtable.sleepUsec(1000000 - *dd.vtable.ddStartOpTime);
    }

    dd.vtable.markDDAvailable();
    dd.vtable.restartMusicThread();
}


void _isPrintfInit() 
{
    #ifdef DEBUGTOOLS
        dd.sISVHandle = dd.vtable.osCartRomInit();
        dd.vtable.osEPiWriteIo(dd.sISVHandle, (u32)&gISVDbgPrnAdrs->put, 0);
        dd.vtable.osEPiWriteIo(dd.sISVHandle, (u32)&gISVDbgPrnAdrs->get, 0);
        dd.vtable.osEPiWriteIo(dd.sISVHandle, (u32)&gISVDbgPrnAdrs->magic, ASCII_TO_U32('I', 'S', '6', '4'));
    #endif
}

void* _is_proutSyncPrintf(void* arg, const char* str, unsigned int count) 
{
    #ifdef DEBUGTOOLS
        u32 data;
        s32 pos;
        s32 start;
        s32 end;

        dd.vtable.osEPiReadIo(dd.sISVHandle, (u32)&gISVDbgPrnAdrs->magic, &data);

        if (data != ASCII_TO_U32('I', 'S', '6', '4')) 
            return (void*)1;

        dd.vtable.osEPiReadIo(dd.sISVHandle, (u32)&gISVDbgPrnAdrs->get, &data);
        pos = data;
        dd.vtable.osEPiReadIo(dd.sISVHandle, (u32)&gISVDbgPrnAdrs->put, &data);
        start = data;
        end = start + count;

        if (end >= 0xFFE0) 
        {
            end -= 0xFFE0;
            if (pos < end || start < pos) 
                return (void*)1;
        } 
        else 
        {
            if (start < pos && pos < end)
                return (void*)1;
        }

        while (count) 
        {
            u32 addr = (u32)&gISVDbgPrnAdrs->data + (start & 0xFFFFFFC);
            s32 shift = ((3 - (start & 3)) * 8);

            if (*str) 
            {
                dd.vtable.osEPiReadIo(dd.sISVHandle, addr, &data);
                dd.vtable.osEPiWriteIo(dd.sISVHandle, addr, (*str << shift) | (data & ~(0xFF << shift)));

                start++;
                if (start >= 0xFFE0) 
                    start -= 0xFFE0;
            }
            count--;
            str++;
        }

        dd.vtable.osEPiWriteIo(dd.sISVHandle, (u32)&gISVDbgPrnAdrs->put, start);

        return (void*)1;
    #endif
}

void is64Printf(const char* fmt, ...) 
{
    #ifdef DEBUGTOOLS
        va_list args;
        va_start(args, fmt);

        dd.funcTablePtr->printf(_is_proutSyncPrintf, NULL, fmt, args);

        va_end(args);
    #endif
}

void ShowFullScreenGraphic(void* graphic, u32 graphicLen)
{
    u8* comprBuf = (u8*)0x80700000;
    dd.funcTablePtr->loadFromDisk(comprBuf, (u32)graphic, graphicLen);
    void* frameBuffer = ddGetCurFrameBuffer(); 
    ddYaz0_Decompress(comprBuf, frameBuffer, graphicLen);
    while (true);
}

void PrintTextLineToFb(u8* frameBuffer, char* msg, int xPos, int yPos, bool fontStyle)
{
    int msgGfxOffs = 0;
    int msgWidth = 0;
    static u8 msgGfxBuf[0x80];

    if (xPos == -1)
    {
        if (fontStyle == 0)
            xPos = SCREEN_WIDTH / 2 - 16 * ddStrlen(msg) / 2;
        else
        {
            for (int i = 0; msg[i] != '\0'; i++)
            {
                u8 chara = msg[i] - 0x20;
                msgWidth += dd.vtable.sFontWidths[chara];
            }

            xPos = SCREEN_WIDTH / 2 - msgWidth / 2;
        }
    }

    if (yPos == -1)
        yPos = SCREEN_HEIGHT / 2 - 8;

    for (int i = 0; msg[i] != '\0'; i++)
    {
        int width = 16;

        if (fontStyle == 0)
            FontLoadChar_64DDIPL(msgGfxBuf, ddGetSJisIndex(msg[i]));
        else
        {
            u8 chara = msg[i] - ' ';
            width = dd.vtable.sFontWidths[chara];
            FontLoadChar_ROM(msgGfxBuf, chara);
        }

        dd.vtable.printCharToFramebuffer(msgGfxBuf, xPos, yPos, 16, 16, 11, frameBuffer, SCREEN_WIDTH);
        xPos += width;
    }
}