#include "diskCode.h"

#include "ddScenes.c"
#include "static_vtables.c"
#include "funcRepl.c"
#include "funcExtend.c"

DDHookTable hookTable = 
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
    .unk_4C                     = { },
    .handleEntranceTriggers     = NULL,
    .setMessageTables           = NULL,
    .unk_5C                     = { },
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
    .hookTablePtr             = (DDHookTable*)NULL,
    .gameVersion              = -1,
    .vtable                   = {},
    .sState                   = {.musicId = -1, .destinationScene = -1, .stateLoadCounter = 0},
};

void* vTableDiskAddrs[] = {&VTABLE_1_0, &VTABLE_1_1, NULL, &VTABLE_1_2};

void Disk_Init(ddFuncPointers* funcTablePtr, DDHookTable* hookTablePtr)
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
        ShowFullScreenGraphic(ERROR_VERSION_YAZ0, ERROR_VERSION_YAZ0_LEN, true);
    else
        dd.funcTablePtr->loadFromDisk(&dd.vtable, (s32)vTableDiskAddrs[dd.gameVersion], sizeof(VersionVTable));

    SaveContext* sContext = dd.funcTablePtr->saveContext;
    sContext->language = LANGUAGE_ENG;
    sContext->gameMode = GAMEMODE_NORMAL;

    if (*(u32*)sContext->unk_1358 == 0)                     // If new save (check using an unused field in the save)
    {
        ddMemcpy(sContext->unk_1358, SAVE_ID, 4);
    }
    else if (ddMemcmp(sContext->unk_1358, SAVE_ID, 4))      // Save from another disk.
        ShowFullScreenGraphic(ERROR_SAVE_YAZ0, ERROR_SAVE_YAZ0_LEN, true);

    _isPrintfInit();
    Functions_ReplaceAll(replFunctions, ARRAY_COUNT(replFunctions));

    is64Printf("64DD Ready!\n"); 
}

void Disk_Destroy()
{
    ShowFullScreenGraphic(PLEASERESET_YAZ0, PLEASERESET_YAZ0_LEN, true);
}

void Disk_GameState(struct GameState* state)
{
    SaveContext* sContext = dd.funcTablePtr->saveContext;
    Input* input = &state->input[0];

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
    func[play->sceneDrawConfig](play);  
    //dd.funcTablePtr->faultDrawText(25, 25, "%x %x %x", *dd.vtable.diskBuffer, lba, offset);

    #ifdef MAP_SELECT
        RestoreMapSelect(play);
    #endif

    #ifdef SAVESTATES
        DoSaveStates(play);
    #endif    

    #ifdef SIGN_CLOCK
        DoClockDisplayOnLinkHouseSign(play);
    #endif

    #ifdef DVDLOGO 
        Draw64DDDVDLogo(play); 
    #endif
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

                    // We're done loading! 
                    dd.funcTablePtr->osSendMesg(&roomCtx->loadQueue, NULL, OS_MESG_NOBLOCK);                                      
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

    #ifdef ARWING
        // Talking to Saria for the first time.
        if (msgC->textId == 0x1002)
        {
            ddMemcpy(font->msgBuf, "ARWING, GO!\x02", 200);
            
            if (dd.play)
                SpawnArwing(dd.play);

            return 1;
        }
    #endif

    #ifdef SIGN_CLOCK
        if (msgC->textId == 0x31F)
        {
            ddMemcpy(font->msgBuf, "00:00:00 is the current real time!\x02", 200);
            return 1;
        }
    #endif

    dd.funcTablePtr->dmaMgrRequestSync(font->msgBuf, (uintptr_t)(font->msgOffset + dd.vtable.ENGLISH_MESSAGE_DATA), font->msgLength); 
    return 1;
}

void Disk_SetMessageTables(struct MessageTableEntry** Japanese, struct MessageTableEntry** English, struct MessageTableEntry** Credits)
{
}

// ===========================================================================================================

#ifdef DVDLOGO

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
    GraphicsContext* gfxCtx = play->state.gfxCtx;
    OPEN_DISPS(gfxCtx, __FILE__, __LINE__);

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
    CLOSE_DISPS(gfxCtx, __FILE__, __LINE__);
}

#endif

#ifdef ARWING
void SpawnArwing(struct PlayState* play)
{
    dd.vtable.audioPlaySfxGeneral(NA_SE_SY_KINSTA_MARK_APPEAR, dd.vtable.gSfxDefaultPos, 4, 
                                  dd.vtable.gSfxDefaultFreqAndVolScale, dd.vtable.gSfxDefaultFreqAndVolScale, dd.vtable.gSfxDefaultReverb);
    
    Player* player = GET_PLAYER(play);
    dd.vtable.actorSpawn(&play->actorCtx, play, ACTOR_EN_CLEAR_TAG, player->actor.world.pos.x,
                                            player->actor.world.pos.y + 50.0f, player->actor.world.pos.z, 0, 0, 0, 0); 
}
#endif

#ifdef SIGN_CLOCK
void DoClockDisplayOnLinkHouseSign(struct PlayState* play)
{
    if (play->msgCtx.textId == 0x31F)
    {
        __LOCTime time;
        dd.vtable.locReadTimer(&time);
        char msgString[12] = "00:00:00";
        dd.vtable.sprintf(msgString, "%02x:%02x:%02x", time.hour, time.minute, time.second);

        for (int i = 0; i < 8; i++)
            FontLoadChar_ROM(&play->msgCtx.font.charTexBuf[i * FONT_CHAR_TEX_SIZE], msgString[i] - 0x20);
    }    
}
#endif

#ifdef MAP_SELECT
void RestoreMapSelect(struct PlayState* play)
{
    Input* input = play->state.input;

    if (CHECK_BTN_ALL(input->cur.button, BTN_Z | BTN_START))
    {
        SET_NEXT_GAMESTATE(&play->state, dd.vtable.mapSelectInit, MapSelectState);
        play->state.running = false;
        return;
    }    
}
#endif

#ifdef SAVESTATES
void DoSaveStates(struct PlayState* play)
{
    Input* input = play->state.input;
    SaveContext* sc = dd.funcTablePtr->saveContext;

    int saveSize =  0x785C8 + sizeof(DDSavedState) + sizeof(gSaveContext) + (RAM_START - (u32)&play) + 0x1610 + 0x100;
    int diskPos = ROM_LENGTH - saveSize;
    ALIGN(diskPos, 32);

    // Save state
    if (CHECK_BTN_ALL(input->press.button, BTN_L))
    {  
        dd.vtable.sleepMsec(100);
        void* frameBuffer = ddGetCurFrameBuffer(); 
        dd.vtable.clearFrameBuffer(frameBuffer);
        PrintTextLineToFb(frameBuffer, SAVING_MSG, -1, SCREEN_HEIGHT / 2 - 16, 1);
        PrintTextLineToFb(frameBuffer, PLEASE_WAIT, -1, SCREEN_HEIGHT / 2 + 16, 1);

        ddMemcpy(&dd.sState.magic, STATE_MAGIC, 16);
        dd.sState.destinationEntrance = sc->save.entranceIndex;
        dd.sState.linkAge = sc->save.linkAge;
        dd.sState.musicId = sc->seqId;
        dd.sState.destinationScene = play->sceneId;
        dd.sState.stateLoadCounter = 0;        

        Disk_Write_MusicSafe(&dd.sState, diskPos, sizeof(DDSavedState));
        diskPos += sizeof(DDSavedState);
        ALIGN(diskPos, 32);

        Disk_Write_MusicSafe(sc, diskPos, sizeof(gSaveContext));
        diskPos += sizeof(gSaveContext);     
        ALIGN(diskPos, 32);       

        u32 plAddr = (u32)play;
        Disk_Write_MusicSafe((void*)(plAddr - 0x1610), diskPos, (RAM_START - (u32)&play) + 0x1610);
        return;
    }

    // Trigger load state
    if (CHECK_BTN_ALL(input->press.button, BTN_R) && !dd.sState.stateLoadCounter && play->transitionTrigger == TRANS_TRIGGER_OFF)
    {
        dd.vtable.sleepMsec(100);
        void* frameBuffer = ddGetCurFrameBuffer();           
        dd.vtable.clearFrameBuffer(frameBuffer);
        PrintTextLineToFb(frameBuffer, CHECKING_MSG, -1, SCREEN_HEIGHT / 2 - 16, 1);
        PrintTextLineToFb(frameBuffer, PLEASE_WAIT, -1, SCREEN_HEIGHT / 2 + 16, 1);   

        Disk_Read_MusicSafe(&dd.sState, diskPos, sizeof(DDSavedState));

        if (ddMemcmp(dd.sState.magic, STATE_MAGIC, 16))
        {
            dd.vtable.sleepMsec(100);
            void* frameBuffer = ddGetCurFrameBuffer();           
            dd.vtable.clearFrameBuffer(frameBuffer);
            PrintTextLineToFb(frameBuffer, NO_SAVE_MSG, -1, -1, 1);

            dd.sState.stateLoadCounter = 0;
            dd.sState.destinationScene = -1;
            dd.vtable.sleepMsec(1000);
            return;
        }

        // If we're not in the destination area already, go there.
        if (sc->save.entranceIndex != dd.sState.destinationEntrance)
        {
            dd.vtable.audio_StopBgmAndFanfare(0);
            play->nextEntranceIndex = dd.sState.destinationEntrance;
            play->transitionTrigger = TRANS_TRIGGER_START; 
            play->transitionMode = TRANS_MODE_INSTANT;
            play->linkAgeOnLoad = dd.sState.linkAge;      
            
            play->envCtx.screenFillColor[0] = play->envCtx.screenFillColor[1] = play->envCtx.screenFillColor[2] = 0;
            play->envCtx.screenFillColor[3] = 255;
            play->envCtx.fillScreen = 1;
            dd.sState.stateLoadCounter = 2;
            sc->forcedSeqId = 0xFFFF;
        }
        else
            dd.sState.stateLoadCounter = 1;
    }    

    // Upon reaching the destination area, load state from disk.
    if (dd.sState.destinationScene == play->sceneId && dd.sState.stateLoadCounter)
    {
        dd.sState.stateLoadCounter--;

        if (dd.sState.stateLoadCounter != 0)
            return;
          
        dd.vtable.sleepMsec(100);
        void* frameBuffer = ddGetCurFrameBuffer();           
        dd.vtable.clearFrameBuffer(frameBuffer);
        PrintTextLineToFb(frameBuffer, LOADING_MSG, -1, SCREEN_HEIGHT / 2 - 16, 1);
        PrintTextLineToFb(frameBuffer, PLEASE_WAIT, -1, SCREEN_HEIGHT / 2 + 16, 1);           

        diskPos += sizeof(DDSavedState); 
        ALIGN(diskPos, 32);

        Disk_Read_MusicSafe(sc, diskPos, sizeof(gSaveContext));
        diskPos += sizeof(gSaveContext);
        ALIGN(diskPos, 32);       

        u32 plAddr = (u32)play;
        Disk_Read_MusicSafe((void*)(plAddr - 0x1610), diskPos, (RAM_START - (u32)&play) + 0x1610);

        dd.funcTablePtr->osWritebackDCacheAll();
        dd.funcTablePtr->osInvalICache((void*)0x80000000, 0x400000);       
        dd.funcTablePtr->osInvalDCache((void*)0x80000000, 0x400000);  
        
        dd.vtable.audio_QueueSeqCmd(dd.sState.musicId);
        dd.sState.destinationScene = -1;
        return;
    }
}
#endif

// ===========================================================================================================

