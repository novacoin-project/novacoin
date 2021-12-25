#!/bin/bash

CROSS=$1
ROOT=$(pwd)

if [[ ! "${CROSS}" =~ ^(aarch64|x86_64) ]]; then
echo "Platform ${CROSS} is not supported"
echo "Expected either aarch64 or x86_64."
exit 1
fi

# Make build directories
mkdir ${ROOT}/${CROSS}-w64-mingw32-build

# Stage directory
mkdir ${ROOT}/${CROSS}-w64-mingw32

# Compile zlib

cd ${ROOT}/${CROSS}-w64-mingw32-build
CC=${CROSS}-w64-mingw32-gcc CXX=${CROSS}-w64-mingw32-g++ ${ROOT}/zlib/configure --prefix=${ROOT}/${CROSS}-w64-mingw32
#make -j 4
make install-libs

# Remove build directore
cd ${ROOT}
rm -rf ${ROOT}/${CROSS}-w64-mingw32-build
