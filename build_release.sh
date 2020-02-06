#!/bin/bash

set -e

make clean
make -j16
mkdir -p release/atmosphere/contents/5600000000000000/flags
mkdir -p release/switch
cp sysmodule/sysmodule.nsp release/atmosphere/contents/5600000000000000/exefs.nsp
touch release/atmosphere/contents/5600000000000000/flags/boot2.flag
cp client/client.nro release/switch/nxcord.nro
cp -r config release
cd release
zip -r release.zip *
mv release.zip ../
cd ..
rm -r release
