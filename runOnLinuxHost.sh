#!/bin/sh

mkdir -p ./Debug
rm Debug/host.elf 2>/dev/null
g++ src/main.cpp -o Debug/host.elf
chmod a+x Debug/host.elf
./Debug/host.elf
