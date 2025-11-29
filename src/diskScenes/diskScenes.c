#include "diskScenes.h"

DD_ROOMS(ddRoomsTest, DD_ROOM(TEST_ROOM0_ZMAP));

DD_SCENES
(
    DD_SCENE(SCENE_MIDOS_HOUSE, ddRoomsTest, TEST_SCENE_ZSCENE, TEST_SCENE_TITLECARD_BIN, SDC_DEFAULT),
);

const s32 ddScenesCount = ARRAY_COUNT(ddScenes);