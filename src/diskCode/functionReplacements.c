#include "diskCode.h"
#include "versionedCode.c"

#define STUB_FUNC(func_name, target_func, ret_type, params, args) \
__attribute__((long_call)) \
ret_type func_name params { \
    return target_func args; \
}

#define REPLACE_FUNC(stub_name, dst_table, len) \
ddMemcpy(&stub_name, (dst_table)[vars.gameVersion], len); \
vars.funcTablePtr->osWritebackDCacheAll(); \
vars.funcTablePtr->osInvalICache((dst_table)[vars.gameVersion], len)

void FontLoadChar_64DDIPL(Font* font, u8 character, u16 codePointIndex)
{
    PlayState* play = vars.play;

    u16 index = ddGetSJisIndex(character + 0x20, false);

    if (index == 0xFFFF)
    {
        s32 offset = character * FONT_CHAR_TEX_SIZE;
        vars.funcTablePtr->dmaMgrRequestSync(&play->msgCtx.font.charTexBuf[codePointIndex], 
                                            (uintptr_t)(fontWidths_Table[vars.gameVersion] + offset), 
                                            FONT_CHAR_TEX_SIZE);
    }
    else
    {
        s32 offset = index * FONT_CHAR_TEX_SIZE;
        vars.funcTablePtr->dmaFromDriveRom(&play->msgCtx.font.charTexBuf[codePointIndex], 
                                           DDROM_FONT_START + offset, 
                                           FONT_CHAR_TEX_SIZE);
    }
}

STUB_FUNC(_Font_LoadChar, FontLoadChar_64DDIPL, void, (Font* font, u8 character, u16 codePointIndex), (font, character, codePointIndex));

