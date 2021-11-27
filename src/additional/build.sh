#!/bin/bash

ROOT=$(pwd)

git submodule update --init --recursive
mkdir $ROOT/IXWebSocket/build
cd $ROOT/IXWebSocket/build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/ ..
make -j 4
make DESTDIR=$ROOT/stage install
cd $ROOT

