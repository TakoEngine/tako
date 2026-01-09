#!/usr/bin/env bash

cd /workspaces/tako
emcmake cmake -G Ninja -B  build-em -DBUILD_SHARED_LIBS=OFF
