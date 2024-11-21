#!/usr/bin/env bash

EM_VERSION='3.1.71'

EMSDK_PATH=$HOME

env
cd $EMSDK_PATH
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

git pull
./emsdk install $EM_VERSION
./emsdk activate $EM_VERSION

source ./emsdk_env.sh

echo "source \"$EMSDK_PATH/emsdk/emsdk_env.sh\"" >> $HOME/.bash_profile

# Workaround for clang-scan-deps
cd $EMSDK_PATH
git clone https://github.com/eliemichel/cpp20-cmake-emscripten-template.git
chmod +x $EMSDK_PATH/cpp20-cmake-emscripten-template/cmake/EmscriptenScanDepsFix/emscan-deps
sed -i "20s%.*%args.append(\"-I$EMSDK_PATH/emsdk/upstream/emscripten/system/include\")%" $EMSDK_PATH/cpp20-cmake-emscripten-template/cmake/EmscriptenScanDepsFix/emscan-deps.py
