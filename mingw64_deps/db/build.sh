#!/bin/bash

CROSS=$1
ROOT=$(pwd)
MUTEX="x86_64/gcc-assembly"

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

if [ "${CROSS}" == "aarch64" ]; then
MUTEX="ARM64/gcc-assembly"
fi

# Make build directories
mkdir ${ROOT}/${CROSS}-w64-mingw32-build

# Stage directory
mkdir ${ROOT}/${CROSS}-w64-mingw32

# Compile BerkeleyDB

cd ${ROOT}/${CROSS}-w64-mingw32-build
CC=${CROSS}-w64-mingw32-gcc CXX=${CROSS}-w64-mingw32-g++ ${ROOT}/libdb/dist/configure --prefix=${ROOT}/${CROSS}-w64-mingw32 --enable-smallbuild --enable-cxx --disable-shared --disable-replication --with-mutex=${MUTEX} --enable-mingw --host=${CROSS}-w64-mingw32
make -j 4 library_build
make library_install

# Remove build directore
cd ${ROOT}
rm -rf ${ROOT}/${CROSS}-w64-mingw32-build
