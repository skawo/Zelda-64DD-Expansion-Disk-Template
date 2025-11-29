#ifndef FUNCEREPL_H
#define FUNCEREPL_H

#include "../../include/n64dd.h"
#include "../../include/save.h"
#include "../../include/message.h"
#include "../../include/gfx.h"
#include "../../include/game.h"
#include "../../include/play_state.h"
#include "../../include/player.h"
#include "../../include/controller.h"
#include "../../include/sfx.h"
#include "../../include/fault.h"
#include "../../include/audio.h"
#include "../../include/map_select_state.h"
#include "../../include/map.h"
#include "../ddTool/ddTool.h"

typedef struct FuncReplacement
{
    void** toReplace;
    void* replaceWith;
} FuncReplacement;

#define FUNC_REPL_ENTRY(replacedFunction, newFunction)      \
{                                                           \
    .toReplace = (void**)&replacedFunction,                 \
    .replaceWith = (void*)&newFunction,                     \
}

#define DD_FUNC_REPLACEMENTS(...)         \
    FuncReplacement replFunctions[] = {   \
        __VA_ARGS__                       \
    }

extern struct DDState dd;
extern FuncReplacement replFunctions[];
extern const s32 replFunctionsCount;

void Functions_ReplaceAll(FuncReplacement* table, int numEntries);
void FontLoadChar_64DDIPL(void* dest, u8 characterIndex);
void FontLoadChar_ROM(void* dest, u8 characterIndex);
void Font_LoadChar_Repl(Font* font, u8 character, u16 codePointIndex);
void TitleCard_InitPlaceName_Repl(PlayState* play, TitleCardContext* titleCtx, void* texture, s32 x, s32 y, s32 width,s32 height, s32 delay);

#endif