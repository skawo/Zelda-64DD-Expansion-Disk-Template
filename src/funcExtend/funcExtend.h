#ifndef FUNCEXTEND_H
#define FUNCEXTEND_H

#include "../include/n64dd.h"
#include "../include/game.h"
#include "../ddTool/ddTool.h"
#include "../diskCode/diskCode.h"

extern struct DDState dd;

void Disk_Load_MusicSafe(void* dest, s32 offset, s32 size);
void Disk_Write_MusicSafe(void* data, u32 diskAddr, u32 len);
void Disk_Write(void* data, u32 diskAddr, u32 len);
void _isPrintfInit();
void* _is_proutSyncPrintf(void* arg, const char* str, unsigned int count);
void is64Printf(const char* fmt, ...) ;
void ShowFullScreenGraphic(void* graphic, u32 graphicLen);
void PrintTextLineToFb(u8* frameBuffer, char* msg, int xPos, int yPos, bool fontStyle);

#endif // FUNCEXTEND_H