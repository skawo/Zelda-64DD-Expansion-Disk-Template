#include "ddScenes.h"

DDRoom ddRoomsTest[] =
{
    DD_ROOM(TEST_ROOM0_ZMAP),
    END_ROOMLIST,
};

DDScene ddScenes[] =
{
    DD_SCENE(SCENE_MIDOS_HOUSE, ddRoomsTest, TEST_SCENE_ZSCENE, TEST_SCENE_TITLECARD_BIN, SDC_DEFAULT),
};