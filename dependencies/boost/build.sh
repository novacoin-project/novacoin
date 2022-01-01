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

if [ "${TARGET_OS}" == "w64-mingw32" ]; then
MINGW32_PARAMS="target-os=windows"
fi

# Create stage directory
mkdir ${ROOT}/${CROSS}

cd ${ROOT}/boost

# Create compiler settings
echo "using gcc : : ${CROSS}-g++ ;" > user-config-${CROSS}.jam

# Build boost
./b2 --user-config=user-config-${CROSS}.jam cxxflags="-fstack-protector-strong -D_FORTIFY_SOURCE=2" linkflags=-fstack-protector-strong --build-type=minimal --layout=system --with-chrono --with-filesystem --with-program_options --with-system --with-thread ${MINGW32_PARAMS} address-model=64 variant=release link=static threading=multi runtime-link=static stage --prefix=${ROOT}/${CROSS} install

cd ${ROOT}

# Remove build directories
rm -rf ${ROOT}/boost/stage ${ROOT}/boost/bin.v2 ${ROOT}/boost/user-config-${CROSS}.jam
