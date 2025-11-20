#include "diskCode.h"
#include "versionedCode.c"

// Define a replacement function stub that wraps a target function
// and create a marker label immediately after it in memory.
#define STUB_FUNC_ARGS(stub_name, target_func, ret_type, params, args) \
    __attribute__((long_call)) \
    ret_type stub_name params { \
        return target_func args; \
    } \
    /* Marker symbol to calculate length of function */ \
    extern u32 stub_name##end; \
    __asm__( \
        ".global " #stub_name "end\n" \
        #stub_name "end:" \
    )

#define STUB_FUNC(stub_name, target_func)           \
    extern void stub_name();                        \
    extern u32 stub_name##end;                      \
    __asm__(                                        \
        ".global " #stub_name "\n"                  \
        ".type " #stub_name ", @function\n"         \
        #stub_name ":\n"                            \
        "    j " #target_func "\n"                  \
        "    nop\n"                                 \
        ".global " #stub_name "end\n"               \
        #stub_name "end:\n"                         \
    )

// Calculate length of a stub function in bytes
#define GET_LEN(stub_name) ((u32)&stub_name##end - (u32)&stub_name)

// Replace the destination function with the stub
#define REPLACE_FUNC(stub_name, dst_table) \
    ddMemcpy(&stub_name, (dst_table)[vars.gameVersion], GET_LEN(stub_name)); \
    vars.funcTablePtr->osWritebackDCacheAll(); \
    vars.funcTablePtr->osInvalICache((dst_table)[vars.gameVersion], GET_LEN(stub_name)); 

void FontLoadChar_64DDIPL(Font* font, u8 character, u16 codePointIndex)
{
    u16 index = ddGetSJisIndex(character + 0x20, false);

    if (index == 0xFFFF)
    {
        s32 offset = character * FONT_CHAR_TEX_SIZE;
        vars.funcTablePtr->dmaMgrRequestSync(&font->charTexBuf[codePointIndex], 
                                            (uintptr_t)(nesFont_Table[vars.gameVersion] + offset), 
                                            FONT_CHAR_TEX_SIZE);
    }
    else
    {
        s32 offset = index * FONT_CHAR_TEX_SIZE;
        vars.funcTablePtr->dmaFromDriveRom(&font->charTexBuf[codePointIndex], 
                                           DDROM_FONT_START + offset, 
                                           FONT_CHAR_TEX_SIZE);
    }
} 

STUB_FUNC(_Font_LoadChar, FontLoadChar_64DDIPL);
