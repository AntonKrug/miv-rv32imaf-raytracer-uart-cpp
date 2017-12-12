#!/bin/sh

mkdir -p ./Debug
rm Debug/host.elf
g++ src/main.cpp -o Debug/host.elf
chmod a+x Debug/host.elf
./Debug/host.elf
