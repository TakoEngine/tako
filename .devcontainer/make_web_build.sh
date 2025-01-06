#!/usr/bin/env bash

cd /workspaces/tako
emcmake cmake -G Ninja -B  build-em -DBUILD_SHARED_LIBS=OFF -DCMAKE_CXX_COMPILER_CLANG_SCAN_DEPS=~/cpp20-cmake-emscripten-template/cmake/EmscriptenScanDepsFix/emscan-deps
