#!/bin/bash

ROOT=$(pwd)

git submodule update --init --recursive
mkdir $ROOT/IXWebSocket/build
cd $ROOT/IXWebSocket/build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/ -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 ..
make -j 4
make DESTDIR=$ROOT/stage install
cd $ROOT

mkdir $ROOT/leveldb/build
cd $ROOT/leveldb/build
git reset --hard 4cb80b7ddce6ff6089b15d8cfebf746fc1572477
cmake -DCMAKE_INSTALL_PREFIX:PATH=/ -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 ..
make -j 4
make DESTDIR=$ROOT/stage install
cd $ROOT
