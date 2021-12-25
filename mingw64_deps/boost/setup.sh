#!/bin/bash

ROOT=$(pwd)
VER=1_78_0

wget -O ${ROOT}/boost_${VER}.tar.gz https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_${VER}.tar.gz
tar -xzf ${ROOT}/boost_${VER}.tar.gz
mv ${ROOT}/boost_${VER} boost

cd ${ROOT}/boost

# Compile b2 tool
./bootstrap.sh

cd ${ROOT}
