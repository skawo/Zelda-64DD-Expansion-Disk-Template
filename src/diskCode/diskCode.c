#include "diskCode.h"
#include "ddScenes.c"
#include "static_vtables.c"
#include "funcRepl.c"

__attribute__((section(".codeHeader")))
char Header[] = "ZELDA_DD";

/* There is padding here in the file */

__attribute__((section(".diskInfo")))
diskInfo diskInfoData = 
{
    .diskStart    = &__Disk_Start,
    .diskEnd      = &__Disk_End,         
    .vramStart    = &__Disk_VramStart,
    .vramEnd      = &__Disk_VramEnd,  
    .hookTablePtr = &hookTable,
    .unk_014      = { 0 }
};

ddHookTable hookTable = 
{
    .diskInit                   = (DiskInitFunc)&__Disk_Init_K1,
    .diskDestroy                = Disk_Destroy,
    .loadRoom                   = Disk_LoadRoom,
    .sceneInit                  = NULL,
    .playInit                   = Disk_PlayInit,
    .playDestroy                = Disk_PlayDestroy,
    .mapDataInit                = NULL,
    .mapDataDestroy             = NULL,
    .mapDataSetDungeons         = NULL,
    .mapExpDestroy              = NULL,
    .mapExpTextureLoadDungeons  = NULL,
    .mapMarkInit                = NULL,
    .mapMarkDestroy             = NULL,
    .pauseMapMarkInit           = NULL,
    .pauseMapMarkDestroy        = NULL,
    .kaleidoInit                = NULL,
    .kaleidoDestroy             = NULL,
    .kaleidoLoadDungeonMap      = NULL,
    .getSceneEntry              = Disk_GetSceneEntry,
    .unk_4C                     = { 0 },
    .handleEntranceTriggers     = NULL,
    .setMessageTables           = NULL,
    .unk_5C                     = { 0 },
    .loadCreditsMsg             = NULL,
#if OOT_PAL
    .loadEnglishMsg             = NULL,
    .loadGermanMsg              = NULL,
    .loadFrenchMsg              = NULL,
#else
    .loadJapaneseMsg            = NULL,
    .loadEnglishMsg             = Disk_GetENGMessage,
#endif
    .sceneDraw                  = Disk_SceneDraw,
    .asyncDma                   = NULL,
    .gameStateUpdate            = Disk_GameState,
    .cutsceneSetScript          = NULL,
};


DDState dd = 
{
    .play                     = (PlayState*)NULL,
    .funcTablePtr             = (ddFuncPointers*)NULL,
    .hookTablePtr             = (ddHookTable*)NULL,
    .gameVersion              = -1,
    .vtable                   = {},
};

void Disk_Init(ddFuncPointers* funcTablePtr, ddHookTable* hookTablePtr)
{
    funcTablePtr->osWritebackDCacheAll();
    funcTablePtr->osInvalICache((void*)RAM_START, RAM_LENGTH);
 
    dd.funcTablePtr = funcTablePtr;
    dd.hookTablePtr = hookTablePtr;    
    
    // Check game version by comparing the address of the funcTablePtr.
    for (int i = NTSC_1_0; i <= NTSC_1_2; i++)
    {
        if (funcTablePtrs[i - 1] == (void*)funcTablePtr)
            dd.gameVersion = i - 1;
    }

    // If no valid version detected, show error screen and hang.
    // Otherwise, load the vTable from disk.
    if (dd.gameVersion < 0)
        ShowErrorScreen(ERROR_VERSION_YAZ0, ERROR_VERSION_YAZ0_LEN);
    else
        dd.funcTablePtr->loadFromDisk(&dd.vtable, (s32)vTableDiskAddrs[dd.gameVersion], sizeof(dd.vtable));

    SaveContext* sContext = dd.funcTablePtr->saveContext;
    sContext->language = LANGUAGE_ENG;
    sContext->gameMode = GAMEMODE_NORMAL;

    if (*(u32*)sContext->unk_1358 == 0)                     // If new save (check using an unused field in the save)
    {
        ddMemcpy(sContext->unk_1358, SAVE_ID, 4);
    }
    else if (ddMemcmp(sContext->unk_1358, SAVE_ID, 4))      // Save from another disk.
        ShowErrorScreen(ERROR_SAVE_YAZ0, ERROR_SAVE_YAZ0_LEN);

    REPLACE_FUNC(dd.vtable.fontLoadChar, Font_LoadChar_Repl);

    _isPrintfInit();
    is64Printf("64DD Ready!\n");
}

void Disk_Destroy()
{
    ShowErrorScreen(PLEASERESET_YAZ0, PLEASERESET_YAZ0_LEN);
}

void Disk_GameState(struct GameState* state)
{
    SaveContext* sContext = dd.funcTablePtr->saveContext;

    // We have no idea of the state the disk has left the game code, and so it's best to ask the player to reset.
    if (dd.play && dd.play->pauseCtx.promptChoice && dd.play->pauseCtx.state == PAUSE_STATE_GAME_OVER_FINISH)
        Disk_Destroy();   
}

void Disk_PlayInit(struct PlayState* play)
{
    dd.play = play;
}

void Disk_PlayDestroy(struct PlayState* play)
{
    dd.play = NULL;
}

void Disk_SceneDraw(struct PlayState* play, SceneDrawConfigFunc* func)
{
    #define __gfxCtx (play->state.gfxCtx)
    
    Input* input = play->state.input;
    func[play->sceneDrawConfig](play);  

    //vars.funcTablePtr->faultDrawText(25, 25, msgString);

    if (play->msgCtx.textId == 0x31F)
    {
        __LOCTime time;
        dd.vtable.locReadTimer(&time);
        char msgString[12] = "00:00:00";
        dd.vtable.sprintf(msgString, "%02x:%02x:%02x", time.hour, time.minute, time.second);

        for (int i = 0; i < 8; i++)
            FontLoadChar_ROM(&play->msgCtx.font, msgString[i] - 0x20, i * FONT_CHAR_TEX_SIZE);
    }

    Draw64DDDVDLogo(play);

    if (dd.funcTablePtr->saveContext->showTitleCard && dd.titleCardAddr)
        dd.play->actorCtx.titleCtx.texture = dd.titleCardAddr;
}

struct SceneTableEntry* Disk_GetSceneEntry(s32 sceneId, struct SceneTableEntry* sceneTable)
{
    for (int i = 0; i < ARRAY_COUNT(ddScenes); i++)
    {
        DDScene* scene = &ddScenes[i];

        if (scene->sceneId == sceneId)
        {
            u8* addr = (u8*)ROOMS_START;

            for (int j = 0; j < MAX_ROOMS; j++)
            {
                DDRoom* entry = &scene->rooms[j];

                if (!entry->diskAddr)
                    break;

                dd.funcTablePtr->loadFromDisk(addr, entry->diskAddr, entry->size);
                addr += entry->size;
            }

            return &scene->entry;
        }
    }

    // Prevent crashing when using the ROM data in 64DD Mode.
    sceneTable[sceneId].unk_12 = 0;
    return &sceneTable[sceneId];
}

void Disk_LoadRoom(struct PlayState* play, struct RoomContext* roomCtx, s32 roomNum)
{
    for (int i = 0; i < ARRAY_COUNT(ddScenes); i++)
    {
        DDScene* scene = &ddScenes[i];

        if (scene->sceneId == play->sceneId)
        {
            u8* addr = (u8*)ROOMS_START;

            for (int j = 0; j < MAX_ROOMS; j++)
            {
                DDRoom* entry = &scene->rooms[j];

                if (j == roomNum)
                {
                    roomCtx->roomRequestAddr = addr;
                    addr += entry->size;

                    u32 titleCardSize = scene->entry.titleFile.vromEnd - scene->entry.titleFile.vromStart;
                    dd.titleCardAddr = titleCardSize ? addr : NULL;
                    dd.funcTablePtr->loadFromDisk(dd.titleCardAddr, scene->entry.titleFile.vromStart, titleCardSize); 

                    dd.funcTablePtr->osSendMesg(&roomCtx->loadQueue, NULL, OS_MESG_NOBLOCK);                  // We're done loading!                     
                    return;
                }

                addr += entry->size;
            }
        }
    } 

    // Regular room load from cartridge.
    u32 size = play->roomList.romFiles[roomNum].vromEnd - play->roomList.romFiles[roomNum].vromStart;
    dd.funcTablePtr->dmaMgrRequestAsync(&roomCtx->dmaRequest, roomCtx->roomRequestAddr,
                                          play->roomList.romFiles[roomNum].vromStart, size, 0, &roomCtx->loadQueue, NULL);
}

s32 Disk_GetENGMessage(struct Font* font)
{
    MessageContext* msgC = (MessageContext*)((u8*)font - offsetof(MessageContext, font));

    // Talking to Saria for the first time.
    if (msgC->textId == 0x1002)
    {
        ddMemcpy("ARWING, GO!\x02", font->msgBuf, 200);
        
        if (dd.play)
            SpawnArwing(dd.play);
    }
    else if (msgC->textId == 0x31F)
    {
        ddMemcpy("00:00:00 is the current real time!\x02", font->msgBuf, 200);
    }
    else
        dd.funcTablePtr->dmaMgrRequestSync(font->msgBuf, (uintptr_t)(font->msgOffset + dd.vtable.ENGLISH_MESSAGE_DATA), font->msgLength); 

    return 1;
}

void Disk_SetMessageTables(struct MessageTableEntry** Japanese, struct MessageTableEntry** English, struct MessageTableEntry** Credits)
{
}

// ===========================================================================================================

void ShowErrorScreen(void* graphic, u32 graphicLen)
{
    u32* viReg = (u32*)K0_TO_K1(VI_ORIGIN_REG);
    u8* frameBuffer = (void*)K0_TO_K1(*viReg);
    u8* comprBuf = (u8*)SEGMENT_STATIC_START;
    dd.funcTablePtr->loadFromDisk(comprBuf, (u32)graphic, graphicLen);
    ddYaz0_Decompress(comprBuf, frameBuffer, graphicLen);
    while (true);
}

void DrawRect(Gfx** gfxp, u8 r, u8 g, u8 b, u32 PosX, u32 PosY, u32 Sizex, u32 SizeY)
{
    Gfx* gfx = *gfxp;

    u16 color5551 = GPACK_RGBA5551(r, g, b, 1);

    gDPPipeSync(gfx++);
    gDPSetFillColor(gfx++, (color5551 << 16) | color5551);
    gDPFillRectangle(gfx++, PosX, PosY, PosX + Sizex, PosY + SizeY);

    *gfxp = gfx;
}

void Draw64DDDVDLogo(struct PlayState* play)
{
    #define __gfxCtx (play->state.gfxCtx)
    Gfx* gfxRef = OVERLAY_DISP;

    // Logo animation state
    static int posX = 100;
    static int posY = 100;
    static int velX = 2;
    static int velY = 2;

    // Logo dimensions
    #define BLOCK_SIZE 7
    #define LOGO_WIDTH (21 * BLOCK_SIZE)  // 5+3+5+5 blocks + 3 gaps
    #define LOGO_HEIGHT (5 * BLOCK_SIZE)

    // Update position
    posX += velX;
    posY += velY;

    // Bounce off edges
    if (posX <= 0 || posX + LOGO_WIDTH >= SCREEN_WIDTH) 
        velX = -velX;
    if (posY <= 0 || posY + LOGO_HEIGHT >= SCREEN_HEIGHT) 
        velY = -velY;

    int currentX = posX;

    // 6
    DrawRect(&gfxRef, 0, 255, 0, currentX, posY + BLOCK_SIZE, BLOCK_SIZE, 4*BLOCK_SIZE);                    // Left vertical
    DrawRect(&gfxRef, 0, 255, 0, currentX + BLOCK_SIZE, posY, 3*BLOCK_SIZE, BLOCK_SIZE);                    // Top horizontal
    DrawRect(&gfxRef, 0, 255, 0, currentX + BLOCK_SIZE, posY + 2*BLOCK_SIZE, 3*BLOCK_SIZE, BLOCK_SIZE);     // Middle horizontal
    DrawRect(&gfxRef, 0, 255, 0, currentX + BLOCK_SIZE, posY + 4*BLOCK_SIZE, 3*BLOCK_SIZE, BLOCK_SIZE);     // Bottom horizontal
    DrawRect(&gfxRef, 0, 255, 0, currentX + 4*BLOCK_SIZE, posY + 3*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);     // Right connector

    // 4
    currentX += 5 * BLOCK_SIZE;
    DrawRect(&gfxRef, 255, 255, 0, currentX, posY, BLOCK_SIZE, 3*BLOCK_SIZE);                               // Left vertical (top 3)
    DrawRect(&gfxRef, 255, 255, 0, currentX + 3*BLOCK_SIZE, posY, BLOCK_SIZE, 5*BLOCK_SIZE);                // Right vertical (full)
    DrawRect(&gfxRef, 255, 255, 0, currentX + BLOCK_SIZE, posY + 2*BLOCK_SIZE, 2*BLOCK_SIZE, BLOCK_SIZE);   // Middle horizontal

    // D
    currentX += 5 * BLOCK_SIZE;
    DrawRect(&gfxRef, 0, 255, 255, currentX, posY, BLOCK_SIZE, 5*BLOCK_SIZE);                               // Left vertical
    DrawRect(&gfxRef, 0, 255, 255, currentX + BLOCK_SIZE, posY, 3*BLOCK_SIZE, BLOCK_SIZE);                  // Top horizontal
    DrawRect(&gfxRef, 0, 255, 255, currentX + BLOCK_SIZE, posY + 4*BLOCK_SIZE, 3*BLOCK_SIZE, BLOCK_SIZE);   // Bottom horizontal
    DrawRect(&gfxRef, 0, 255, 255, currentX + 4*BLOCK_SIZE, posY + BLOCK_SIZE, BLOCK_SIZE, 3*BLOCK_SIZE);   // Right vertical

    // D
    currentX += 5 * BLOCK_SIZE;
    DrawRect(&gfxRef, 255, 0, 255, currentX, posY, BLOCK_SIZE, 5*BLOCK_SIZE);                               // Left vertical
    DrawRect(&gfxRef, 255, 0, 255, currentX + BLOCK_SIZE, posY, 3*BLOCK_SIZE, BLOCK_SIZE);                  // Top horizontal
    DrawRect(&gfxRef, 255, 0, 255, currentX + BLOCK_SIZE, posY + 4*BLOCK_SIZE, 3*BLOCK_SIZE, BLOCK_SIZE);  // Bottom horizontal
    DrawRect(&gfxRef, 255, 0, 255, currentX + 4*BLOCK_SIZE, posY + BLOCK_SIZE, BLOCK_SIZE, 3*BLOCK_SIZE);  // Right vertical

    OVERLAY_DISP = gfxRef;
}

void SpawnArwing(struct PlayState* play)
{
    dd.vtable.audioPlaySfxGeneral(NA_SE_SY_KINSTA_MARK_APPEAR, dd.vtable.gSfxDefaultPos, 4, 
                                  dd.vtable.gSfxDefaultFreqAndVolScale, dd.vtable.gSfxDefaultFreqAndVolScale, dd.vtable.gSfxDefaultReverb);
    
    Player* player = GET_PLAYER(play);
    dd.vtable.actorSpawn(&play->actorCtx, play, ACTOR_EN_CLEAR_TAG, player->actor.world.pos.x,
                                            player->actor.world.pos.y + 50.0f, player->actor.world.pos.z, 0, 0, 0, 0); 
}

#define gISVDbgPrnAdrs ((ISVDbg*)0xB3FF0000)
#define ASCII_TO_U32(a, b, c, d) ((u32)((a << 24) | (b << 16) | (c << 8) | (d << 0)))

void _isPrintfInit() 
{
    dd.sISVHandle = dd.vtable.osCartRomInit();
    dd.vtable.osEPiWriteIo(dd.sISVHandle, (u32)&gISVDbgPrnAdrs->put, 0);
    dd.vtable.osEPiWriteIo(dd.sISVHandle, (u32)&gISVDbgPrnAdrs->get, 0);
    dd.vtable.osEPiWriteIo(dd.sISVHandle, (u32)&gISVDbgPrnAdrs->magic, ASCII_TO_U32('I', 'S', '6', '4'));
}

void* _is_proutSyncPrintf(void* arg, const char* str, unsigned int count) 
{
    u32 data;
    s32 pos;
    s32 start;
    s32 end;

    dd.vtable.osEPiReadIo(dd.sISVHandle, (u32)&gISVDbgPrnAdrs->magic, &data);

    if (data != ASCII_TO_U32('I', 'S', '6', '4')) 
        return (void*)1;

    dd.vtable.osEPiReadIo(dd.sISVHandle, (u32)&gISVDbgPrnAdrs->get, &data);
    pos = data;
    dd.vtable.osEPiReadIo(dd.sISVHandle, (u32)&gISVDbgPrnAdrs->put, &data);
    start = data;
    end = start + count;

    if (end >= 0xFFE0) 
    {
        end -= 0xFFE0;
        if (pos < end || start < pos) 
            return (void*)1;
    } 
    else 
    {
        if (start < pos && pos < end)
            return (void*)1;
    }

    while (count) 
    {
        u32 addr = (u32)&gISVDbgPrnAdrs->data + (start & 0xFFFFFFC);
        s32 shift = ((3 - (start & 3)) * 8);

        if (*str) 
        {
            dd.vtable.osEPiReadIo(dd.sISVHandle, addr, &data);
            dd.vtable.osEPiWriteIo(dd.sISVHandle, addr, (*str << shift) | (data & ~(0xFF << shift)));

            start++;
            if (start >= 0xFFE0) 
                start -= 0xFFE0;
        }
        count--;
        str++;
    }

    dd.vtable.osEPiWriteIo(dd.sISVHandle, (u32)&gISVDbgPrnAdrs->put, start);

    return (void*)1;
}

void is64Printf(const char* fmt, ...) 
{
    va_list args;
    va_start(args, fmt);

    dd.funcTablePtr->printf(_is_proutSyncPrintf, NULL, fmt, args);

    va_end(args);
}