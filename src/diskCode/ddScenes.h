#include "diskCode.h"

typedef struct DDRoom
{
    uintptr_t diskAddr;
    uintptr_t size;
} DDRoom;

typedef struct DDScene
{
    SceneTableEntry entry;
    int sceneId;
    DDRoom* rooms;
} DDScene;

#define END_ROOMLIST {(uintptr_t)NULL, (u32)NULL}
#define MAX_ROOMS 32

#define DD_ROOM(file) \
    { \
        .diskAddr = (uintptr_t)(file), \
        .size     = (file##_LEN) \
    }

#define DD_SCENE_FILE(file) \
    { \
        .vromStart = (uintptr_t)(file), \
        .vromEnd   = (uintptr_t)((file) + (file##_LEN)) \
    }    

#define DD_SCENE(_sceneId, _roomList, _sceneFile, _titleFile, _drawFunc) \
    { \
        .entry = \
        { \
            .sceneFile  = DD_SCENE_FILE(_sceneFile), \
            .titleFile  = DD_SCENE_FILE(_titleFile), \
            .drawConfig = (_drawFunc), \
            .unk_12     = 1, \
        }, \
        .sceneId = (_sceneId), \
        .rooms   = (_roomList), \
    }