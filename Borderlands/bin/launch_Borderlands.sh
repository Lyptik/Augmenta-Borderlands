#!/bin/sh

# You need to have screen installed

# Get correct paths
ORIG_PATH=`pwd`
cd `dirname $0`
SCRIPT_PATH=`pwd`
cd $ORIG_PATH

cd $SCRIPT_PATH &&
screen -S borderlands -d -m env DISPLAY=:0 $SCRIPT_PATH/Borderlands

