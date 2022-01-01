#!/bin/bash

TARGET_CPU=$1
TARGET_OS=$2
ROOT=$(pwd)

if [[ ! "${TARGET_CPU}" =~ ^(aarch64|x86_64) ]]; then
echo "Platform ${TARGET_CPU} is not supported"
echo "Expected either aarch64 or x86_64."
exit 1
fi

if [[ ! "${TARGET_OS}" =~ ^(w64\-mingw32|linux\-gnu) ]]; then
echo "Operation sysrem ${TARGET_OS} is not supported"
echo "Expected either w64-mingw32 or linux-gnu."
exit 1
fi

# Cross-building prefix
CROSS=${TARGET_CPU}-${TARGET_OS}

if [[ ! $(which ${CROSS}-gcc) ]]; then
echo "Target C compiler ${CROSS}-gcc is not found"
exit 1
fi

if [[ ! $(which ${CROSS}-g++) ]]; then
echo "Target C++ compiler ${CROSS}-g++ is not found"
exit 1
fi

if [[ ! $(which make) ]]; then
echo "make is not installed, please install buld-essential package"
exit 1
fi

if [ "${TARGET_CPU}" == "aarch64" ]; then
MUTEX="ARM64/gcc-assembly"
fi

# Make build directoriy
cp -r ${ROOT}/zlib ${ROOT}/${CROSS}-build-zlib

# Stage directory
mkdir ${ROOT}/${CROSS}

# Compile zlib
cd ${ROOT}/${CROSS}-build-zlib
export CC=${CROSS}-gcc
export CXX=${CROSS}-g++
export LD=${CROSS}-ld
export RANLIB=${CROSS}-ranlib
export CFLAGS="-fstack-protector-all -D_FORTIFY_SOURCE=2"
export CXXFLAGS=${CFLAGS}
export LDFLAGS="-fstack-protector-all"
${ROOT}/zlib/configure --prefix=${ROOT}/${CROSS} --static
make -j 4

# Install zlib to our cross-tools directory
make install

# Remove build directory
cd ${ROOT}
rm -rf ${ROOT}/${CROSS}-build-zlib
