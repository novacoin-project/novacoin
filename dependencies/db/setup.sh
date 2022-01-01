#!/bin/bash

if [[ ! $(which wget) ]]; then
    echo "wget is not installed"
    exit -1
fi

if [[ ! $(which tar) ]]; then
    echo "tar is not installed"
    exit -1
fi

if [[ ! $(which sed) ]]; then
    echo "sed is not installed"
    exit -1
fi

wget https://fossies.org/linux/misc/db-18.1.40.tar.gz
tar -xzf db-18.1.40.tar.gz
mv db-18.1.40 libdb
sed -i "s/WinIoCtl.h/winioctl.h/g" libdb/src/dbinc/win_db.h
