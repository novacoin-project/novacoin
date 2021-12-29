#!/bin/bash

PREVDIR=$(pwd)
SCRIPT=$(readlink -f $0)
ROOT=`dirname $SCRIPT`

cd $ROOT/IXWebSocket
git reset --hard 8c15405
patch -s -p1 < ../cerrno_include.patch
git -c user.name='CryptoManiac' -c user.email='balthazar@yandex.ru' commit -a -m 'Fix cerrno'
cd $PREVDIR
