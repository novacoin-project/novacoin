#!/bin/bash

if [[ ! $(which git) ]]; then
    echo "git is not installed"
    exit -1
fi

git clone -b 5.15.2 https://github.com/qt/qtbase.git
git clone -b 5.15.2 https://github.com/qt/qttools.git
git clone -b 5.15.2 https://github.com/qt/qttranslations.git

