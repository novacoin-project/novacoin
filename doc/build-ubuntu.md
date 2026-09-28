# Building GUI and headless client for Ubuntu Linux

## Preparations

First step, install all dependencies:

    sudo apt install build-essential libdb++-dev libssl-dev cmake gcc-10 qtdeclarative5-dev qttools5-dev libboost-all-dev libpng-dev libdeflate-dev git
	
Clone, build and install Open Quantum Safe library:

    git clone https://github.com/open-quantum-safe/liboqs

Create build directory:

    mkdir liboqs/build && cd liboqs/build

Configure build files:

    CC=gcc-10 cmake -DOQS_MINIMAL_BUILD="SIG_falcon_512" -DBUILD_SHARED_LIBS=OFF -DOQS_BUILD_ONLY_LIB=ON -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_POSITION_INDEPENDENT_CODE=ON ..

Compile:

    make -j 4
	
Install:

    sudo make install && cd


Then, clone repository recursively:

    git clone --recursive https://github.com/novacoin-project/novacoin

## Building GUI client

Create build directory:

    mkdir build_qt && cd build_qt

Configure build files:

    cmake ../novacoin

Compile:

    make -j 4

After everything will be done the resulting novacoin-qt executable will be created in your build directory.

## Building headless client

It's almost identical to steps for GUI client.

Create build directory:

    cd && mkdir build_daemon && cd build_daemon

Configure build files:

    cmake ../novacoin/src

Compile:

    make -j 4

The resulting novacoind executable will be created in the build directory.
