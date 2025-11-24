This repository allows you to create Zelda: Ocarina of Time Expansion Disks using C.

Provided is a general working template with rudimentary error handling and a few examples of what can be done using the game hooks.

This code has been created using the excellent <a href="https://github.com/LuigiBlood/zelda_dawn_dusk">Zelda: Dawn and Dusk</a> mod by Captain Seedy-Eye and LuigiBlood as a guide.

# Building
Requires Python 3 and make.

1. Extract the toolchain archives found in tool/gcc.
2. Run build.py found in the repository root.

Options may be passed to build.sh:
   * "USA", "USA-DEV", "USA-D64", "JPN", "JPN-DEV", "JPN-D64" - Build target
   * "-clean" - Runs make clean before building the disk
   * "-skipfs" - Foregoes building the filesystem (to speed up compilation once a lot of files have been added).
   * "-fs" - Only generates the filesystem (for when a new file is added, to just generate the necessary symbols).

# Adding files
Simply drop in .zscene, .zmap, .bin or .yaz files into the filesystem directory.
(Additional extensions can be added by editing the <i>build.sh</i> script)

After running .build.py, headers will be generated in <i>include/fileHeaders</i> which should be included in the filesystem.c file.

Afterwards, files can be loaded from the disk using the appropriate function (see the <i>ShowErrorScreen</i> function in <i>diskCode.c</i> for an example).

There also exists a pre-made function to replace arbitrary scenes in the game. The scenes and their rooms need to be defined in ddScenes.c

# Calling original functions
Provided is a semi-automatic way to obtain the addresses for functions and variables in all three NTSC revisions of Ocarina of Time.
You can find all the available symbols in the symbols1_x.h files found in the src folder. 

Edit vtabledef.json to add the function name and signature without the _1_x suffix.

After recompiling, that function will be available for use in the dd.vtable struct. The vtable is loaded at the end of the Disk_Init function.
This also works for global variables of all kinds.

# Replacing original functions
You can overwrite any of the game's functions by adding your new function in funcRepl.c. The new function should use all of the original's arguments,
in the same order. The new function can be any size.

After implementing the new function, add an entry to the DD_FUNC_REPLACEMENTS macro, passing the vtable entry of the function to replace, and the name of the new function:

<i>FUNC_REPL_ENTRY(dd.vtable.someFunctionEntry, Function_Name)</i>
