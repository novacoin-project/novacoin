# Cross-build dependencies for Windows x86-64 and Windows ARM64

## Requirements

Please use llvm-mingw compiler which can be downloaded here:

    https://github.com/mstorsjo/llvm-mingw/releases

You will also need to install CMake 3.20 or newer to compile Qt libraries. Simplest way to do this is to use python repositories:

    sudo apt install python3-pip
    pip3 install cmake

Unpack compiler binaries to suitable directory (/opt is preferred) and ensure that its /bin subdirectory is mentioned in your PATH. You will also need to add /home/user/.local/bin to PATH as well.

## Setting up

   Every subdirectory contains a setup.sh script which can be used to download source code and create source trees.

## Building

   Just execute ./build.sh script for corresponding dependency using either x86_64 or aarch64 as argument. It will build binaries automatically.
