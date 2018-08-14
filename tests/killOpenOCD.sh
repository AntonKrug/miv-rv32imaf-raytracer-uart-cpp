#!/bin/bash

echo "Will kill all existing OpenOCD and fpServer instances"

function kill_all_existing {
    # kill it even from other users, as only instance of OpenOCD can run at the same time
    # make sure you do not run these scripts in paraler 

    PIDS=`pidof $1`
    if [ "$PIDS" ];
    then
        if [[ "$(command -v sudo)" ]];
        then
            kill $PIDS
        else
            sudo kill $PIDS
        fi 
    fi
    sleep 3

    PIDS=`pidof $1`
    if [ "$PIDS" ];
    then
        # agressive kill
        if [[ "$(command -v sudo)" ]];
        then
            kill -9 $PIDS
        else
            sudo kill -9 $PIDS
        fi 
    fi
    sleep 2
    echo "Is $1 still running? PIDs:"
    pidof $1
}

echo "Making sure NO OpenOCD/fpServer is running"

kill_all_existing openocd
kill_all_existing fpServer
