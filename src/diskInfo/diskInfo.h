#ifndef DISKINFO_H 
#define DISKINFO_H

#include "../diskCode/diskCode.h"

typedef struct diskInfo 
{
    /* 0x0000 */ char diskHeader[8];
    /* 0x0008 */ char padding[0x1058];
    /* 0x1060 */ void* diskStart;     
    /* 0x1064 */ void* diskEnd;       
    /* 0x1068 */ void* vramStart; 
    /* 0x106C */ void* vramEnd; 
    /* 0x1070 */ ddHookTable* hookTablePtr;
    /* 0x1074 */ char padding2[0x104];
} diskInfo;

extern void* __Disk_Start;
extern void* __Disk_End;
extern void* __Disk_VramStart;
extern void* __Disk_VramEnd;
extern ddHookTable hookTable;

#endif // DISKINFO_H