#!/bin/sh

# Script that installs needed addons

# Errors out
set -e

[ "$1" = "-h" ] || [ "$1" = "--help" ] || [ $# -gt 0 ] && { echo "Usage : $0 [ --version ]" && exit 1; }

# Get correct paths
ORIG_PATH=`pwd`
cd `dirname $0`
SCRIPT_PATH=`pwd`
cd $ORIG_PATH

# Install ofxSyphon
ADDONS=ofxSyphon
cd $SCRIPT_PATH/../../../addons/ &&
if [ ! -d $ADDONS ]; then
	git clone https://github.com/astellato/ofxSyphon.git &&
	cd $ADDONS &&
	git checkout master
fi

# Install ofxAugmenta
ADDONS=ofxAugmenta
cd $SCRIPT_PATH/../../../addons/ &&
if [ ! -d $ADDONS ]; then
	git clone https://github.com/Theoriz/ofxAugmenta.git &&
	cd $ADDONS &&
	git checkout master
fi

# Install ofxLibsndfileRecorder
ADDONS=ofxLibsndfileRecorder
cd $SCRIPT_PATH/../../../addons/ &&
if [ ! -d $ADDONS ]; then
	git clone https://github.com/angeloseme/ofxLibsndfileRecorder.git &&
	cd $ADDONS &&
	git checkout master
fi

cd $ORIG_PATH &&
echo "Done!"
