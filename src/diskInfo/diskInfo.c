#include "diskInfo.h"

__attribute__((section(".diskInfo")))
diskInfo diskInfoData = 
{
    .diskHeader                 = "ZELDA_DD",
    .padding[0 ... 0x1057]      = 0xFF,
    .diskStart                  = &__Disk_Start,
    .diskEnd                    = &__Disk_End,         
    .vramStart                  = &__Disk_VramStart,
    .vramEnd                    = &__Disk_VramEnd,  
    .hookTablePtr               = &hookTable,
    .padding2[0 ... 0x103]       = 0,
};