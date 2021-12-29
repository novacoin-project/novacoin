# Sample toolchain file for building for Windows from an Ubuntu Linux system.
#
# Typical usage:
#    *) install cross compiler: `sudo apt-get install mingw-w64`
#    *) cd build
#    *) CROSS=x86_64 TOOLCHAIN_ROOT=/opt DEP_ROOT=./mingw64_deps cmake -DCMAKE_TOOLCHAIN_FILE=~/mingw-w64.cmake ..

set(CMAKE_SYSTEM_NAME Windows)
set(TOOLCHAIN_PREFIX $ENV{CROSS}-w64-mingw32)

# cross compilers to use for C, C++ and Fortran
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

# target environment on the build host system
set(CMAKE_FIND_ROOT_PATH 
    $ENV{TOOLCHAIN_ROOT}/${TOOLCHAIN_PREFIX}
    $ENV{DEP_ROOT}/boost/${TOOLCHAIN_PREFIX}
    $ENV{DEP_ROOT}/db/${TOOLCHAIN_PREFIX}
    $ENV{DEP_ROOT}/openssl/${TOOLCHAIN_PREFIX}
    $ENV{DEP_ROOT}/qt/${TOOLCHAIN_PREFIX}
    $ENV{DEP_ROOT}/zlib/${TOOLCHAIN_PREFIX}
)

set(BerkeleyDB_INC $ENV{DEP_ROOT}/db/${TOOLCHAIN_PREFIX}/include)
set(BerkeleyDB_LIBS $ENV{DEP_ROOT}/db/${TOOLCHAIN_PREFIX}/lib)

# modify default behavior of FIND_XXX() commands
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
