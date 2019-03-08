#!/bin/sh
#obsidian build script -- i'm too lazy to setup make

###############################################################################################################
# default configuration: release
libs="-Bstatic -lassimp"
flags="-O3"
defines=""
files="main.cxx vertex.cxx displaylist.cxx"
output="build/obsidian"
###############################################################################################################

if [[ $1 == "clean" ]]; then
    rm -r build/
    exit 0
fi

function compile () {
    echo "[INFO] Building obsidian..."
    rm *.o &> /dev/null
    rm -r build &> /dev/null
    mkdir build &> /dev/null
    g++ $flags $files -o $output $libs $defines
    if [ $? == 0 ]; then
        echo "[✓] Build succeeded!"
    else
        echo "[X] Build failed!"
    fi
}

###############################################################################################################
# release builds
###############################################################################################################

if [[ $1 == "release" ]]; then
    compile
fi

# fallback

if [[ $1 == "" ]]; then
    compile
fi

if [[ $1 == "conf_release" ]]; then # collision config
    conf_compile
fi

###############################################################################################################
# debug builds (no output)
###############################################################################################################

if [[ $1 == "debug" ]]; then
    defines=""
    flags="-g"
    compile
fi

if [[ $1 == "conf_debug" ]]; then # collision config
    defines=""
    flags="-g"
    conf_compile
    exit 0
fi
