#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $SCRIPT_DIR

mkdir -p ./Debug-native
rm Debug-native/host.elf 2>/dev/null
g++ ./src/main.cpp ./tests/test-utils.c -DEXIT_FROM_THE_INFINITE_LOOP -o Debug-native/host.elf
chmod a+x Debug-native/host.elf
./Debug-native/host.elf
