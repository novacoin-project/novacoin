#!/bin/bash

CROSS=$1
ROOT=$(pwd)

if [[ ! "${CROSS}" =~ ^(aarch64|x86_64) ]]; then
echo "Platform ${CROSS} is not supported"
echo "Expected either aarch64 or x86_64."
exit 1
fi

if [[ ! $(which ${CROSS}-w64-mingw32-clang) ]]; then
echo "llvm-mingw is not installed, please download it from https://github.com/mstorsjo/llvm-mingw/releases"
exit 1
fi

if [[ ! $(which make) ]]; then
echo "make is not installed, please install buld-essential package"
exit 1
fi

# Make build directories
cp -r ${ROOT}/zlib ${ROOT}/${CROSS}-w64-mingw32-build-zlib
mkdir ${ROOT}/${CROSS}-w64-mingw32-build-libpng

# Stage directory
mkdir ${ROOT}/${CROSS}-w64-mingw32

# Compile zlib
cd ${ROOT}/${CROSS}-w64-mingw32-build-zlib
perl -i -pe "s,(PREFIX =)\$,\$1 ${CROSS}-w64-mingw32-," win32/Makefile.gcc
make -j 4 -f win32/Makefile.gcc

# Install zlib to our cross-tools directory
make install DESTDIR=${ROOT}/${CROSS}-w64-mingw32 INCLUDE_PATH=/include LIBRARY_PATH=/lib BINARY_PATH=/bin -f win32/Makefile.gcc

# Remove build directory
cd ${ROOT}
rm -rf ${ROOT}/${CROSS}-w64-mingw32-build-zlib

# Compile libpng
cd ${ROOT}/${CROSS}-w64-mingw32-build-libpng
export CFLAGS="-I${ROOT}/${CROSS}-w64-mingw32/include"
export LDFLAGS="-I${ROOT}/${CROSS}-w64-mingw32/lib"
${ROOT}/libpng/configure --prefix=${ROOT}/${CROSS}-w64-mingw32 --target=${CROSS}-w64-mingw32
make -j 4
make install

# Remove build directory
cd ${ROOT}
rm -rf ${ROOT}/${CROSS}-w64-mingw32-build-libpng
