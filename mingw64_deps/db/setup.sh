#!/bin/bash

wget https://fossies.org/linux/misc/db-18.1.40.tar.gz
tar -xzf db-18.1.40.tar.gz
mv db-18.1.40 libdb
sed -i "s/WinIoCtl.h/winioctl.h/g" libdb/src/dbinc/win_db.h
