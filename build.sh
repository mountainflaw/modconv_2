#!/bin/sh
#obsidian build script -- i'm too lazy to setup make

if [[ $1 == "clean" ]]; then
    rm -r build/
    exit 0
fi

# debug build (no output)

function compile () {
    echo "building"
    rm *.o
    rm -r build
    mkdir build
    g++ $flags main.cxx vertex.cxx displaylist.cxx -o build/obsidian -lassimp $defines
    if [ $? == 0 ]; then
        echo "[✓] Build succeeded!"
    else
        echo "[X] Build failed!"
    fi
}

if [[ $1 == "debug" ]]; then
    defines=""
    flags="-g"
    compile
fi
