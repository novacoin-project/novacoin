#!/bin/bash

TARGET_CPU=$1
TARGET_OS=$2
ROOT=$(pwd)

if [[ ! "${TARGET_CPU}" =~ ^(aarch64|x86_64) ]]; then
echo "Platform ${TARGET_CPU} is not supported"
echo "Expected either aarch64 or x86_64."
exit 1
fi

if [[ ! "${TARGET_OS}" =~ ^(w64\-mingw32|linux\-gnu) ]]; then
echo "Operation sysrem ${TARGET_OS} is not supported"
echo "Expected either w64-mingw32 or linux-gnu."
exit 1
fi

# Cross-building prefix
CROSS=${TARGET_CPU}-${TARGET_OS}

if [[ ! $(which ${CROSS}-gcc) ]]; then
echo "Target C compiler ${CROSS}-gcc is not found"
exit 1
fi

if [[ ! $(which ${CROSS}-g++) ]]; then
echo "Target C++ compiler ${CROSS}-g++ is not found"
exit 1
fi

if [[ ! $(which make) ]]; then
echo "make is not installed, please install buld-essential package"
exit 1
fi

if [ "${TARGET_OS}" == "w64-mingw32" ]; then
    TARGET_PARAMS="win32-clang-g++ -no-freetype"
else
    if [ "${TARGET_CPU}" == "aarch64" ]; then
        TARGET_PARAMS="linux-aarch64-gnu-g++"
    else
        TARGET_PARAMS="linux-g++-64"
    fi
fi

# Make build directories
mkdir ${ROOT}/${CROSS}-build
mkdir ${ROOT}/${CROSS}-build-qttools
mkdir ${ROOT}/${CROSS}-build-qttranslations
mkdir ${ROOT}/${CROSS}-build-qtdeclarative

# Stage directory
mkdir ${ROOT}/${CROSS}

# Compile Qt

cd ${ROOT}/${CROSS}-build
export CFLAGS="-fstack-protector-all -D_FORTIFY_SOURCE=2"
export CXXFLAGS=${CFLAGS}
export LDFLAGS="-fstack-protector-all"
${ROOT}/qtbase/configure -xplatform ${TARGET_PARAMS} --hostprefix=${ROOT}/${CROSS} --prefix=${ROOT}/${CROSS} -device-option CROSS_COMPILE=${CROSS}- -release -confirm-license -no-compile-examples -no-cups -no-egl -no-eglfs -no-evdev -no-icu -no-iconv -no-libproxy -no-openssl -no-reduce-relocations -no-schannel -no-sctp -no-sql-db2 -no-sql-ibase -no-sql-oci -no-sql-tds -no-sql-mysql -no-sql-odbc -no-sql-psql -no-sql-sqlite  -no-sql-sqlite2 -no-system-proxies -no-use-gold-linker -no-zstd -nomake examples -nomake tests -nomake tools -opensource -qt-libpng -qt-pcre -qt-zlib -static -no-feature-bearermanagement -no-feature-colordialog -no-feature-concurrent -no-feature-dial -no-feature-ftp -no-feature-http -no-feature-image_heuristic_mask -no-feature-keysequenceedit -no-feature-lcdnumber -no-feature-networkdiskcache -no-feature-networkproxy -no-feature-pdf -no-feature-printdialog -no-feature-printer -no-feature-printpreviewdialog -no-feature-printpreviewwidget -no-feature-sessionmanager -no-feature-socks5 -no-feature-sql -no-feature-sqlmodel -no-feature-statemachine -no-feature-syntaxhighlighter -no-feature-textbrowser -no-feature-textmarkdownwriter -no-feature-textodfwriter -no-feature-topleveldomain -no-feature-udpsocket -no-feature-undocommand -no-feature-undogroup -no-feature-undostack -no-feature-undoview -no-feature-vnc -no-feature-xml -no-dbus -no-opengl
make -j 4
make install
cd ${ROOT}
rm -rf ${ROOT}/${CROSS}-build

# Compile Qt tools

cd ${ROOT}/${CROSS}-build-qttools
${ROOT}/${CROSS}/bin/qmake ${ROOT}/qttools
make -j 4
make DESTDIR=${ROOT}/${CROSS} install
cd ${ROOT}
rm -rf ${ROOT}/${CROSS}-build-qttools

# Compile translations

cd ${ROOT}/${CROSS}-build-qttranslations
${ROOT}/${CROSS}/bin/qmake ${ROOT}/qttranslations
make -j 4
make DESTDIR=${ROOT}/${CROSS} install
cd ${ROOT}
rm -rf ${ROOT}/${CROSS}-build-qttranslations

# Compile qtdeclarative

cd ${ROOT}/${CROSS}-build-qtdeclarative
${ROOT}/${CROSS}/bin/qmake ${ROOT}/qtdeclarative
make -j 4
make DESTDIR=${ROOT}/${CROSS} install
cd ${ROOT}
rm -rf ${ROOT}/${CROSS}-build-qtdeclarative
