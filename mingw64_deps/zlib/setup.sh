#!/bin/bash

if [[ ! $(which git) ]]; then
    echo "git is not installed"
    exit -1
fi

git clone  https://github.com/madler/zlib
