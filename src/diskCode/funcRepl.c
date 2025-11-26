#include "funcRepl.h"

void Functions_ReplaceAll(FuncReplacement* table, int numEntries)
{
    for (int i = 0; i < numEntries; i++)
    {
        FuncReplacement entry = table[i];

        u32* targetFunc = (u32*) *entry.toReplace;
        void* replacement = entry.replaceWith;

        is64Printf("Replacing %08X -> %08X\n", targetFunc, replacement);

        // Create a J instruction to the replacement function + NOP in the delay slot
        targetFunc[0] = 0x08000000 | (((u32)replacement >> 2) & 0x03FFFFF);
        targetFunc[1] = 0x00000000;

        dd.funcTablePtr->osWritebackDCacheAll();
        dd.funcTablePtr->osInvalICache(targetFunc, 8);
    }
}


// ==============================================================================

void FontLoadChar_64DDIPL(void* dest, u8 characterIndex)
{
    s32 offset = characterIndex * FONT_CHAR_TEX_SIZE;
    dd.funcTablePtr->dmaFromDriveRom(dest, DDROM_FONT_START + offset, FONT_CHAR_TEX_SIZE);    
}

void FontLoadChar_ROM(void* dest, u8 characterIndex)
{
    s32 offset = characterIndex * FONT_CHAR_TEX_SIZE;
    dd.funcTablePtr->dmaMgrRequestSync(dest, (uintptr_t)(dd.vtable.ENGLISH_FONT + offset), FONT_CHAR_TEX_SIZE);  
}

void Font_LoadChar_Repl(Font* font, u8 character, u16 codePointIndex)
{
    u16 index = ddGetSJisIndex(character + 0x20);

    if (index == 0xFFFF)
        FontLoadChar_ROM(&font->charTexBuf[codePointIndex], character);
    else
        FontLoadChar_64DDIPL(&font->charTexBuf[codePointIndex], index);
} 

void TitleCard_InitPlaceName_Repl(PlayState* play, TitleCardContext* titleCtx, void* texture, s32 x, s32 y, s32 width,s32 height, s32 delay) 
{
    SceneTableEntry* loadedScene = play->loadedScene;
    u32 size = loadedScene->titleFile.vromEnd - loadedScene->titleFile.vromStart;

    if ((size != 0) && (size <= 0x1000 * LANGUAGE_MAX)) 
    {
        if (loadedScene->unk_12)
            dd.funcTablePtr->loadFromDisk(texture, loadedScene->titleFile.vromStart, size);
        else
            dd.funcTablePtr->dmaMgrRequestSync(texture, loadedScene->titleFile.vromStart, size);
    }

    titleCtx->texture = texture;
    titleCtx->x = x;
    titleCtx->y = y;
    titleCtx->width = width;
    titleCtx->height = height;
    titleCtx->durationTimer = 80;
    titleCtx->delayTimer = delay;
}

DD_FUNC_REPLACEMENTS
(
    FUNC_REPL_ENTRY(dd.vtable.titleCard_initPlaceName, TitleCard_InitPlaceName_Repl),  
    #ifdef DDIPL_FONT
        FUNC_REPL_ENTRY(dd.vtable.fontLoadChar, Font_LoadChar_Repl),  
    #endif           
);