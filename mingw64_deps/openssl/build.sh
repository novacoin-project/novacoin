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

# Compile BerkeleyDB

cd ${ROOT}/${CROSS}-w64-mingw32-build
#CC=${CROSS}-w64-mingw32-gcc CXX=${CROSS}-w64-mingw32-g++ 
${ROOT}/openssl/Configure --cross-compile-prefix=${CROSS}-w64-mingw32- --prefix=${ROOT}/${CROSS}-w64-mingw32 no-shared no-asm mingw64 --api=1.1.1
make -j 4 build_libs
make install_dev

# Create symlink for compatibility
cd ${ROOT}/${CROSS}-w64-mingw32
ln -s lib64 lib

# Remove build directore
cd ${ROOT}
rm -rf ${ROOT}/${CROSS}-w64-mingw32-build
