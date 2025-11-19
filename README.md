This repository allows you to create Zelda: Ocarina of Time Expansion Disks using C.

Provided is a general working template with rudimentary error handling and a few examples of what can be done using the game hooks.

This code has been created using the excellent <a href="https://github.com/LuigiBlood/zelda_dawn_dusk">Zelda: Dawn and Dusk</a> mod by Captain Seedy-Eye and LuigiBlood as a guide.

# Building
Requires Python 3 and WSL/Linux.

1. Extract the toolchain archives found in tool/gcc.
2. Run build.sh found in the repository root.

Options may be passed to build.sh:
   * "USA", "USA-DEV", "USA-D64", "JPN", "JPN-DEV", "JPN-D64" - Build target
   * "clean" - Runs make clean before building the disk
   * "nofs" - Foregoes building the filesystem (to speed up compilation once a lot of files have been added).

# Adding files
Simply drop in .zscene, .zmap, .bin or .yaz files into the filesystem directory.
(Additional extensions can be added by editing <i>tools/hConv.py</i>)

After running .build.sh, headers will be generated in <i>include/fileHeaders</i> which should be included in the filesystem.c file.

Afterwards, files can be loaded from the disk using the appropriate function (see the <i>Disk_Init</i> function in <i>diskCode.c</i> for an example).

There also exists a pre-made function to replace arbitrary scenes in the game. The scenes and their rooms need to be defined in ddScenes.c
