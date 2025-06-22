#!/usr/bin/env bash

NINJA_VERSION=1.12.1

mkdir /tmpninja
cd /tmpninja
wget https://github.com/ninja-build/ninja/releases/download/v${NINJA_VERSION}/ninja-linux.zip
unzip ninja-linux.zip
cp ninja /usr/bin/
rm -rf /tmpninja
