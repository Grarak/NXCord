#!/bin/bash

set -e

rm -rf build
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../DevkitA64Libnx.cmake ..
make -j16
cd ..
mkdir -p release/atmosphere/contents/5600000000000000/flags
mkdir -p release/switch
cp build/client.nro release/switch/nxcord.nro
cp build/sysmodule.nsp release/atmosphere/contents/5600000000000000/exefs.nsp
touch release/atmosphere/contents/5600000000000000/flags/boot2.flag
cd release
zip -r release.zip *
mv release.zip ../
cd ..
rm -r release
