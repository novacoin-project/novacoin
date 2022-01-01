#!/bin/bash

if [[ ! $(which git) ]]; then
    echo "git is not installed"
    exit -1
fi

git clone -b openssl-3.0 https://github.com/openssl/openssl
