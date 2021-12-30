#!/bin/bash

realpath() {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

PREVDIR=$(pwd)
SCRIPT=$(realpath $0)
ROOT=`dirname $SCRIPT`

cd $ROOT/IXWebSocket
git reset --hard 8c15405
patch -s -p1 < ../cerrno_include.patch
git -c user.name='CryptoManiac' -c user.email='balthazar@yandex.ru' commit -a -m 'Fix cerrno'
cd $PREVDIR
