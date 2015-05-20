#!/bin/sh

# You need to have screen installed

# Get correct paths
ORIG_PATH=`pwd`
cd `dirname $0`
SCRIPT_PATH=`pwd`
cd $ORIG_PATH

cd $SCRIPT_PATH &&

screen -S jackd -d -m env DISPLAY=:0 jackd -P70 -p16 -t2000 -d alsa -dhw:1 -p 128 -n 3 -r 44100 -s

screen -S borderlands -d -m env DISPLAY=:0 $SCRIPT_PATH/Borderlands

