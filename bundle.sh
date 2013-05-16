#!/bin/bash

BUNDLE_ROOT=../bundles
if [ $# -ne 1 ]
then
    echo "Usage: `basename $0` <TAG> [bundle-root]"
    exit $E_BADARGS
fi

if [ "$2" != "" ]; then
BUNDLE_ROOT="$2"
fi

TAG=$1

echo "Tagging with: ${TAG}"
git tag $TAG

BUNDLE_DIR=$BUNDLE_ROOT/$TAG
mkdir -p $BUNDLE_DIR

cp boldhumanoid $BUNDLE_DIR
cp *.ini $BUNDLE_DIR
cp -r www $BUNDLE_DIR
cp motion_4096.bin $BUNDLE_DIR
cp libwebsockets.so* $BUNDLE_DIR

