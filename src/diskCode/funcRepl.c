#include "diskCode.h"

// Define a function stub as __target_func that wraps the target_func.
// Create a marker label immediately after it in memory.
#define STUB_FUNC(target_func)                      \
    extern void __##target_func(void);              \
    extern unsigned int __##target_func##end;       \
    __asm__(                                        \
        ".global __" #target_func "\n"              \
        ".type __" #target_func ", @function\n"     \
        "__" #target_func ":\n"                     \
        "    j " #target_func "\n"                  \
        "    nop\n"                                 \
        ".global __" #target_func "end\n"           \
        "__" #target_func "end:\n"                  \
    )

// Calculate length of a stub function in bytes
#define GET_LEN(target_func) ((u32)&__##target_func##end - (u32)&__##target_func)

// Replace the destination function with the stub
#define REPLACE_FUNC(replacedFunction, newFunction) ReplaceFunc(replacedFunction, &__##newFunction, GET_LEN(newFunction))

void ReplaceFunc(void* replacedFunction, void* newFunction, int functionLen)
{
    ddMemcpy(newFunction, replacedFunction, functionLen);
    dd.funcTablePtr->osWritebackDCacheAll();
    dd.funcTablePtr->osInvalICache(replacedFunction, functionLen);    
}

void FontLoadChar_64DDIPL(Font* font, u8 characterIndex, u16 codePointIndex)
{
    s32 offset = characterIndex * FONT_CHAR_TEX_SIZE;
    dd.funcTablePtr->dmaFromDriveRom(&font->charTexBuf[codePointIndex], 
                                        DDROM_FONT_START + offset, 
                                        FONT_CHAR_TEX_SIZE);    
}

void FontLoadChar_ROM(Font* font, u8 characterIndex, u16 codePointIndex)
{
    s32 offset = characterIndex * FONT_CHAR_TEX_SIZE;
    dd.funcTablePtr->dmaMgrRequestSync(&font->charTexBuf[codePointIndex], 
                                        (uintptr_t)(dd.vtable.ENGLISH_FONT + offset), 
                                        FONT_CHAR_TEX_SIZE);  
}

void Font_LoadChar_Repl(Font* font, u8 character, u16 codePointIndex)
{
    u16 index = ddGetSJisIndex(character + 0x20, false);

    if (index == 0xFFFF)
        FontLoadChar_ROM(font, index, codePointIndex);
    else
        FontLoadChar_64DDIPL(font, index, codePointIndex);
} 

STUB_FUNC(Font_LoadChar_Repl);
