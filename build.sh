#!/bin/sh

VERSION_PARAM="USA"
MAKE_FS="-fs"
CLEAN="noclean"
MAKE_JUST_FS="-notjustfs"

while [ "$#" -gt 0 ]; do
    case "$1" in
        -nfs|--nfs|nfs|nofs|-nofs|--nofs)
            MAKE_FS="nofs"
            ;;
        -just-fs|just-fs|justfs|-justfs)
            MAKE_JUST_FS="-justfs"
            ;;            
        -clean|--clean|-c|--c|c|clean)
            CLEAN="-c"
            ;;
        -*)
            echo "Unknown option: $1"
            exit 1
            ;;
        *)
            VERSION_PARAM="$1"
            ;;
    esac
    shift
done

OS_TYPE=$(uname -s 2>/dev/null || echo Windows)

if [ "$OS_TYPE" = "Linux" ]; then
    BASS_CMD="./bass"
    PY3_CMD="python3"
else
    BASS_CMD="./bass.exe"
    PY3_CMD="py -3"
fi

if [ "$CLEAN" = "-c" ]; then
    MAKE_CMD="make clean && make"
else
    MAKE_CMD="make"
fi

if [ "$MAKE_FS" = "-fs" ] || [ "$MAKE_JUST_FS" = "-justfs" ]; then
    $PY3_CMD tool/hConv.py filesystem src/filesystem --priority error_screens/Error_IPL.yaz0 --extensions zmap,zscene,bin,yaz0,tbl
    
    if [ "$MAKE_JUST_FS" = "-justfs" ]; then
        exit 
    fi
fi

printf "Compiling...\n"
cd src
eval "$MAKE_CMD" "$VERSION_PARAM" || { echo "Build failed"; exit 1; }
cd ..
printf "OK.\n\n"