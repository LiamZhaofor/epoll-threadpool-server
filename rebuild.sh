#!/usr/bin/env bash
set -e

if [ ! -d build ]; then
    mkdir build
    cd build
    cmake ..
    cd ..
fi

cmake --build build
./build/server