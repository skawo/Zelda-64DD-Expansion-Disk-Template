#ifndef FUNCEXTEND_H
#define FUNCEXTEND_H

#include "../common.h"
#include "../include/n64dd.h"
#include "../include/game.h"
#include "../ddTool/ddTool.h"
#include "../diskCode/diskCode.h"

extern struct DDState dd;
extern u8 diskAccessIcon[];

#define DISK_ACCESS_ICON_X 16
#define DISK_ACCESS_ICON_Y 16
#define DISK_ACCESS_ICON_XPOS SCREEN_WIDTH - 30
#define DISK_ACCESS_ICON_YPOS SCREEN_HEIGHT - 30

void Disk_Load(void* dest, s32 offset, s32 size);
void Disk_Write(void* data, u32 diskAddr, u32 len);
void _isPrintfInit();
void* _is_proutSyncPrintf(void* arg, const char* str, unsigned int count);
void ShowFullScreenGraphic(void* graphic, u32 graphicLen);
void PrintTextLineToFb(u8* frameBuffer, char* msg, int xPos, int yPos, bool fontStyle);
void* getCurLatchedFbuf();
void DrawDiskLoadIcon();

#ifdef DEBUGTOOLS
    void is64Printf(const char* fmt, ...);
#else
    #define is64Printf(...) ((void)0)
#endif

#endif // FUNCEXTEND_H