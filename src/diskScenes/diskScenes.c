#include "diskScenes.h"

DD_ROOMS(ddRoomsTest, DD_ROOM(TEST_ROOM0_ZMAP));

DD_SCENES
(
    DD_SCENE(SCENE_MIDOS_HOUSE, ddRoomsTest, TEST_SCENE_ZSCENE, TEST_SCENE_TITLECARD_BIN, SDC_DEFAULT),
);

DD_CUTSCENES
(
    DD_CUTSCENE(SCENE_ZORAS_RIVER, CUTSCENEZORARIVER_BIN),
);

const s32 ddScenesCount = ARRAY_COUNT(ddScenes);
const s32 ddCutscenesCount = ARRAY_COUNT(ddCutscenes);