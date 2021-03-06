#!/bin/bash

SC_DIR="${SC_DIR:-/opt/microsemi/softconsole/}"

TESTS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"


$TESTS_DIR/killOpenOCD.sh

$SC_DIR/openocd/bin/openocd -f board/microsemi-riscv.cfg -c "echo openocd-started" &
sleep 4 # give enough time for hardware to init
echo "OpenOCD should be launched, start the gdb now:"

echo "Go into first Debug folder I can find (make sure to run clean before this so there are no other folders present)"
# One of these below should success
cd $TESTS_DIR/../Debug*
cd $TESTS_DIR/../Release*
$SC_DIR/riscv-unknown-elf-gcc/bin/riscv64-unknown-elf-gdb -x ../tests/gdb-test-checksum *.elf

# Store the exit code
RESULT=$?

# Kill OpenOCD first
$TESTS_DIR/killOpenOCD.sh


# Cascade the gdb pass exit code out as a pass and all others as fails
echo "Got $RESULT"
if [ $RESULT == 149 ];
then
    echo "Exiting as PASS"
    exit 0
fi


if [ $RESULT == 150 ];
then
    echo "Exiting as FAILURE on test failed (the checksum didn't matched)"
else
    echo "Exiting as FAILURE on the process failed (openocd/build/gdb)"
fi
exit -1

