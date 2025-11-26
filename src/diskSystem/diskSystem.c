#include "diskSystem.h"

#define NUM_SECTORS                 85
#define DISK_ZONES                  16
#define NUM_HEADS                   2

#define NUM_DISK_SYSTEM_LBAS        24
#define NUM_DISK_HEADERS            14
#define NUM_DISK_IDS                10

#if !defined(DISKRELEASE_DEV)
    #define DEFECT_TRACKS_PER_ZONE      12
#else
    #define DEFECT_TRACKS_PER_ZONE      10
#endif

#define MAGIC_DEV 0x00000000
#define MAGIC_USA 0x2263EE56
#define MAGIC_JPN 0xE848D316

#define LEO_DISK_TYPE_0 0
#define LEO_DISK_TYPE_1 1
#define LEO_DISK_TYPE_2 2
#define LEO_DISK_TYPE_3 3
#define LEO_DISK_TYPE_4 4
#define LEO_DISK_TYPE_5 5
#define LEO_DISK_TYPE_6 6

#define DISK_TYPE LEO_DISK_TYPE_4
#define IPL_LBA_LOAD_AMOUNT 0x000A

#if defined(DISKRELEASE_DEV) || !defined(DISKFORMAT_NDD)
    #define DISK_REGION MAGIC_DEV
#elif defined(DISKREGION_USA)
    #define DISK_REGION MAGIC_USA
#elif defined(DISKREGION_JPN)
    #define DISK_REGION MAGIC_JPN
#else
    #define DISK_REGION MAGIC_DEV
#endif

#if defined(DISKFORMAT_NDD)
    #define DISK_FORMAT 0x10
    #define DISK_TYPE_VAL (0x10 + DISK_TYPE)
#else
    #define DISK_FORMAT 0x00
    #define DISK_TYPE_VAL DISK_TYPE
#endif

#if !defined(DISKRELEASE_DEV) && defined(DISKFORMAT_NDD)
    #if (DISK_TYPE == LEO_DISK_TYPE_0)
        #define ROM_LBA_END     0x589
        #define RAM_LBA_START   0x58A
    #elif (DISK_TYPE == LEO_DISK_TYPE_1)
        #define ROM_LBA_END     0x7AD
        #define RAM_LBA_START   0x7AE
    #elif (DISK_TYPE == LEO_DISK_TYPE_2)
        #define ROM_LBA_END     0x9D1
        #define RAM_LBA_START   0x9D2
    #elif (DISK_TYPE == LEO_DISK_TYPE_3)
        #define ROM_LBA_END     0xBF5
        #define RAM_LBA_START   0xBF6
    #elif (DISK_TYPE == LEO_DISK_TYPE_4)
        #define ROM_LBA_END     0xE19
        #define RAM_LBA_START   0xE1A
    #elif (DISK_TYPE == LEO_DISK_TYPE_5)
        #define ROM_LBA_END     0xFF7
        #define RAM_LBA_START   0xFF8
    #elif (DISK_TYPE == LEO_DISK_TYPE_6)
        #define ROM_LBA_END     0x10C3
        #define RAM_LBA_START   0x10C4
    #endif
    #define RAM_LBA_END         0x10C3
    #define PADDING_END         0xFFFF
#elif !defined(DISKRELEASE_DEV)
    #define ROM_LBA_END         0xFFFF
    #define RAM_LBA_START       0xFFFF
    #define RAM_LBA_END         0xFFFF
    #define PADDING_END         0x0000
#endif

#if defined(DISKREGION_USA)
    #define DISK_ID_STR "EZLE"
#elif defined(DISKREGION_JPN)
    #define DISK_ID_STR "EZLJ"
#else
    #define DISK_ID_STR "    "
#endif

#define FACTORY_LINE "HOMEBREW"
#ifndef TIMESTAMP
    #define TIMESTAMP "00000000"
#endif
#define COMPANY_CODE "01"
#define RESERVED_STRING "NOTURA"

extern void* __IPL_Entry;

#define DISK_DATA_INIT                      \
{                                           \
    .region = DISK_REGION,                  \
    .diskFormat = DISK_FORMAT,              \
    .diskType = DISK_TYPE_VAL,              \
    .loadAmount = IPL_LBA_LOAD_AMOUNT,      \
    .defectTrackDataOffsets = {0},          \
    ._padding_FFFFFFFF = 0xFFFFFFFF,        \
    .loadVramAddress = (void*)&__IPL_Entry, \
    .defectTrackData = {0},                 \
    DISK_DATA_LBA_FIELDS                    \
}

#if !defined(DISKRELEASE_DEV)
    #define DISK_DATA_LBA_FIELDS            \
        .romLBAEnd = ROM_LBA_END,           \
        .ramLBAStart = RAM_LBA_START,       \
        .ramLBAEnd = RAM_LBA_END,           \
        ._padding_end = PADDING_END,
#else
    #define DISK_DATA_LBA_FIELDS
#endif

#define DISK_ID_INIT                        \
{                                           \
    .diskID = DISK_ID_STR,                  \
    .diskVersion = 0,                       \
    .diskNumber = 0,                        \
    .useMFS = false,                        \
    .diskUse = 0,                           \
    .factoryLine = FACTORY_LINE,            \
    .productionTime = TIMESTAMP,            \
    .companyCode = COMPANY_CODE,            \
    .reserved = RESERVED_STRING             \
}

typedef struct BadSectorOffset
{
    u8 start;
    u8 end;
} BadSectorOffset;

typedef struct DiskData
{
    u32 region;
    u8 diskFormat;
    u8 diskType;
    u16 loadAmount;
    BadSectorOffset defectTrackDataOffsets[DISK_ZONES / NUM_HEADS];
    u32 _padding_FFFFFFFF;
    void* loadVramAddress;
    u8 defectTrackData[DISK_ZONES][DEFECT_TRACKS_PER_ZONE]; 
#if !defined(DISKRELEASE_DEV)
    u16 romLBAEnd;
    u16 ramLBAStart;
    u16 ramLBAEnd;
    u16 _padding_end;
#endif

} DiskData;

typedef struct DiskDataBlock
{
    DiskData diskData[NUM_SECTORS];
    #if defined(DISKRELEASE_DEV)
        u8 padding[3400];
    #endif    
} DiskDataBlock;

typedef struct DiskID
{
    char diskID[4];
    u8 diskVersion;
    u8 diskNumber;
    bool useMFS;
    u8 diskUse;
    char factoryLine[8];
    char productionTime[8];
    char companyCode[2];
    char reserved[6];
    u8 padding[200];
} DiskID;

typedef struct DiskHeader
{
    #if defined(DISKFORMAT_NDD)    
        DiskDataBlock diskDataBlock[NUM_DISK_HEADERS];
        DiskID diskID[NUM_DISK_IDS * NUM_SECTORS];
    #else
        DiskData diskData;
        u8 padding[0x100 - sizeof(DiskData)];
        DiskID diskID;
        u8 padding2[0x200 - sizeof(DiskID) - 0x100];
    #endif

} DiskHeader;

__attribute__((section(".diskHeader")))
DiskHeader header = {
    #if defined(DISKFORMAT_NDD)
        .diskDataBlock[0 ... NUM_DISK_HEADERS - 1] = 
        {
            .diskData[0 ... NUM_SECTORS - 1] = DISK_DATA_INIT
        },
        .diskID[0 ... (NUM_DISK_IDS * NUM_SECTORS - 1)] = DISK_ID_INIT
    #else
        .diskData = DISK_DATA_INIT,
        .diskID = DISK_ID_INIT
    #endif
};
