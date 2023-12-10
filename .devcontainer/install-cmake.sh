#!/usr/bin/env bash

CMAKE_VERSION=3.28.0

mkdir /tmpcmake
cd /tmpcmake
wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz
cd /usr
tar --strip-components=1 -xzf /tmpcmake/cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz
rm -rf /tmpcmake