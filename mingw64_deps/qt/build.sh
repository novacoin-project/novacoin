#!/bin/bash

CROSS=$1
ROOT=$(pwd)

if [[ ! "${CROSS}" =~ ^(aarch64|x86_64) ]]; then
echo "Platform ${CROSS} is not supported"
echo "Expected either aarch64 or x86_64."
exit 1
fi

# Make build directories
mkdir ${ROOT}/${CROSS}-w64-mingw32-build
mkdir ${ROOT}/${CROSS}-w64-mingw32-build-qttools
mkdir ${ROOT}/${CROSS}-w64-mingw32-build-qttranslations
mkdir ${ROOT}/${CROSS}-w64-mingw32-build-qtdeclarative

# Stage directory
mkdir ${ROOT}/${CROSS}-w64-mingw32

# Compile Qt

cd ${ROOT}/${CROSS}-w64-mingw32-build
${ROOT}/qtbase/configure -xplatform win32-clang-g++ --hostprefix=${ROOT}/${CROSS}-w64-mingw32 --prefix=${ROOT}/${CROSS}-w64-mingw32 -device-option CROSS_COMPILE=${CROSS}-w64-mingw32- -release -confirm-license -no-compile-examples -no-cups -no-egl -no-eglfs -no-evdev -no-icu -no-iconv -no-libproxy -no-openssl -no-reduce-relocations -no-schannel -no-sctp -no-sql-db2 -no-sql-ibase -no-sql-oci -no-sql-tds -no-sql-mysql -no-sql-odbc -no-sql-psql -no-sql-sqlite  -no-sql-sqlite2 -no-system-proxies -no-use-gold-linker -no-zstd -nomake examples -nomake tests -nomake tools -opensource -qt-libpng -qt-pcre -qt-zlib -static -no-feature-bearermanagement -no-feature-colordialog -no-feature-concurrent -no-feature-dial -no-feature-ftp -no-feature-http -no-feature-image_heuristic_mask -no-feature-keysequenceedit -no-feature-lcdnumber -no-feature-networkdiskcache -no-feature-networkproxy -no-feature-pdf -no-feature-printdialog -no-feature-printer -no-feature-printpreviewdialog -no-feature-printpreviewwidget -no-feature-sessionmanager -no-feature-socks5 -no-feature-sql -no-feature-sqlmodel -no-feature-statemachine -no-feature-syntaxhighlighter -no-feature-textbrowser -no-feature-textmarkdownwriter -no-feature-textodfwriter -no-feature-topleveldomain -no-feature-udpsocket -no-feature-undocommand -no-feature-undogroup -no-feature-undostack -no-feature-undoview -no-feature-vnc -no-feature-xml -no-dbus -no-opengl -no-freetype
make -j 4
make install
cd ${ROOT}
rm -rf ${ROOT}/${CROSS}-w64-mingw32-build

# Compile Qt tools

cd ${ROOT}/${CROSS}-w64-mingw32-build-qttools
${ROOT}/${CROSS}-w64-mingw32/bin/qmake ${ROOT}/qttools
make -j 4
make DESTDIR=${ROOT}/${CROSS}-w64-mingw32 install
cd ${ROOT}
rm -rf ${ROOT}/${CROSS}-w64-mingw32-build-qttools

# Compile translations

cd ${ROOT}/${CROSS}-w64-mingw32-build-qttranslations
${ROOT}/${CROSS}-w64-mingw32/bin/qmake ${ROOT}/qttranslations
make -j 4
make DESTDIR=${ROOT}/${CROSS}-w64-mingw32 install
cd ${ROOT}
rm -rf ${ROOT}/${CROSS}-w64-mingw32-build-qttranslations

# Compile qtdeclarative

cd ${ROOT}/${CROSS}-w64-mingw32-build-qtdeclarative
${ROOT}/${CROSS}-w64-mingw32/bin/qmake ${ROOT}/qtdeclarative
make -j 4
make DESTDIR=${ROOT}/${CROSS}-w64-mingw32 install
cd ${ROOT}
rm -rf ${ROOT}/${CROSS}-w64-mingw32-build-qtdeclarative
