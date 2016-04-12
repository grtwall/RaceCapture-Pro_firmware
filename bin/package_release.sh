#!/bin/bash

[ -n "$DEBUG" ] && set -x

TMP_DIR=$(mktemp -d)
PLATFORM=$1
VERSION=$2
BUILD_DIR=$3

RELEASE_DIR="RaceCapturePro_${PLATFORM}"
RELEASE_DIR_PATH="${TMP_DIR}/${RELEASE_DIR}"
RELEASE_NAME=RaceCapturePro_${PLATFORM}_${VERSION}

mkdir -p $RELEASE_DIR_PATH

cp CHANGELOG $RELEASE_DIR_PATH
cp $BUILD_DIR/HOW_TO_FLASH.txt $RELEASE_DIR_PATH
cp $BUILD_DIR/main.ihex $RELEASE_DIR_PATH/$RELEASE_NAME.ihex

(cd $TMP_DIR; zip -FSr $RELEASE_NAME.zip $RELEASE_DIR)

cp $TMP_DIR/$RELEASE_NAME.zip .
[ -z "$DEBUG" ] && rm -rf $TMP_DIR
exit 0
