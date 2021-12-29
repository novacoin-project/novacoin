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

# Create stage directory
mkdir ${ROOT}/${CROSS}-w64-mingw32

cd ${ROOT}/boost

# Create compiler settings
echo "using gcc : : ${CROSS}-w64-mingw32-g++ ;" > user-config-${CROSS}.jam

# Build boost
./b2 --user-config=user-config-${CROSS}.jam --build-type=minimal --layout=system --with-chrono --with-filesystem --with-program_options --with-system --with-thread target-os=windows address-model=64 variant=release link=static threading=multi runtime-link=static stage --prefix=${ROOT}/${CROSS}-w64-mingw32 install

cd ${ROOT}

# Remove build directories
rm -rf ${ROOT}/boost/stage ${ROOT}/boost/bin.v2
