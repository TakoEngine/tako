#!/usr/bin/env bash

EM_VERSION='3.1.69'

env
cd $HOME
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

git pull
./emsdk install $EM_VERSION
./emsdk activate $EM_VERSION

source ./emsdk_env.sh

echo 'source "$HOME/emsdk/emsdk_env.sh"' >> $HOME/.bash_profile

# Workaround for clang-scan-deps
cd $HOME
git clone https://github.com/eliemichel/cpp20-cmake-emscripten-template.git
chmod +x $HOME/cpp20-cmake-emscripten-template/cmake/EmscriptenScanDepsFix/emscan-deps
sed -i '20s%.*%args.append("-I/home/tako/emsdk/upstream/emscripten/system/include")%' $HOME/cpp20-cmake-emscripten-template/cmake/EmscriptenScanDepsFix/emscan-deps.py
