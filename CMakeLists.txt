cmake_minimum_required(VERSION 3.4.1)

##
## mkdir build && cd build
##
## cmake -DBerkeleyDB_ROOT:STRING=/opt/homebrew/Cellar/berkeley-db@4/4.8.30 ..
## cmake -DUSE_ASM=1 ..
## cmake -DUSE_SSE2 ..
## cmake -DBerkeleyDB_INC:STRING=/usr/include -DBerkeleyDB_LIBS:STRING=/usr/lib/aarch64-linux-gnu -DUSE_SSE2 -DUSE_LEVELDB ..
##


project(novacoin-qt VERSION 0.5.9 LANGUAGES C CXX ASM)

if (APPLE)
enable_language(OBJCXX)
endif()

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set(CMAKE_C_COMPILER_LAUNCHER ccache)
    set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
endif(CCACHE_FOUND)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS LinguistTools Core Widgets Gui Network Qml REQUIRED)

set (CMAKE_C_FLAGS "-flto -O3 -fPIC -Wno-deprecated -fstack-protector-all -Wstack-protector -D_FORTIFY_SOURCE=2")
set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fexceptions -frtti")

set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost REQUIRED COMPONENTS atomic chrono filesystem program_options system thread)
set(OPENSSL_USE_STATIC_LIBS ON)
find_package(OpenSSL REQUIRED)

add_subdirectory(src/additional/libqrencode)
add_subdirectory(src/additional/IXWebSocket)

# Set default root path for berkeley db
if (NOT BerkeleyDB_ROOT)
set(BerkeleyDB_ROOT /usr)
endif()

# Set default include path for berkeley db
if (NOT BerkeleyDB_INC)
set(BerkeleyDB_INC ${BerkeleyDB_ROOT}/include)
endif()

if (NOT EXISTS ${BerkeleyDB_INC}/db_cxx.h)
    message(FATAL_ERROR "Unable to find db_cxx.h header file in ${BerkeleyDB_INC} directory. Try to specify either include directory path via BerkeleyDB_INC or berkeley db root path via BerkeleyDB_ROOT variable.")
endif()

# Set default library path for berkeley db
if (NOT BerkeleyDB_LIBS)
set(BerkeleyDB_LIBS ${BerkeleyDB_ROOT}/lib)
endif()

set(generic_sources 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/bitcoin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/bitcoingui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/intro.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/transactiontablemodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/addresstablemodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/optionsdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/sendcoinsdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/coincontroldialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/coincontroltreewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/addressbookpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/signverifymessagedialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/aboutdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/editaddressdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/bitcoinaddressvalidator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/trafficgraphwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/mintingfilterproxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/mintingtablemodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/mintingview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/kernelrecord.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/alert.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sync.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/netbase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ntp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/key.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/script.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/miner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/init.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/net.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stun.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/irc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/checkpoints.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/addrman.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/db.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/walletdb.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/clientmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/guiutil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/transactionrecord.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/optionsmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/monitoreddatamapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/transactiondesc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/transactiondescdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/bitcoinstrings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/bitcoinamountfield.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wallet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/keystore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/transactionfilterproxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/transactionview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/walletmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bitcoinrpc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rpcdump.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rpcnet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rpcmining.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rpcwallet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rpcblockchain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rpcrawtransaction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/overviewpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/csvmodelwriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/crypter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/sendcoinsentry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/qvalidatedlineedit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/bitcoinunits.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/qvaluecombobox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/askpassphrasedialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/notificator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/qtipcserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/rpcconsole.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/noui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel_worker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/multisigaddressentry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/multisiginputentry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/multisigdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/secondauthdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/qrcodedialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base58.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ipcollector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/intro.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/coincontroldialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/sendcoinsdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/addressbookpage.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/signverifymessagedialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/aboutdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/editaddressdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/transactiondescdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/overviewpage.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/sendcoinsentry.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/askpassphrasedialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/rpcconsole.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/optionsdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/multisigaddressentry.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/multisiginputentry.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/multisigdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/secondauthdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/qrcodedialog.ui
)

qt6_create_translation(bitcoin_en_qm ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/locale/bitcoin_en.ts)
qt6_create_translation(bitcoin_ru_qm ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/locale/bitcoin_ru.ts)
qt6_create_translation(bitcoin_uk_qm ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/locale/bitcoin_uk.ts)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/qt/bitcoin.qrc bitcoin.qrc)

list(APPEND ALL_SOURCES ${generic_sources} bitcoin.qrc)
list(APPEND ALL_LIBRARIES ${Boost_LIBRARIES} ixwebsocket OpenSSL::Crypto qrencode Qt6::Gui Qt6::Widgets Qt6::Core Qt6::Network)

if (WIN32)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/qt/res/bitcoin-qt.rc bitcoin-qt.rc)
    list(APPEND ALL_SOURCES bitcoin-qt.rc)
endif()

if (APPLE)
    list(APPEND ALL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/res/icons/bitcoin.icns ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/macdockiconhandler.mm ${CMAKE_CURRENT_SOURCE_DIR}/src/qt/macnotificationhandler.mm)
    list(APPEND ALL_LIBRARIES "-framework Foundation -framework ApplicationServices -framework AppKit")
endif()

# Try various libdb library file extensions
if (EXISTS ${BerkeleyDB_LIBS}/libdb_cxx.a)
    list(APPEND ALL_LIBRARIES ${BerkeleyDB_LIBS}/libdb_cxx.a)
elseif(EXISTS ${BerkeleyDB_LIBS}/libdb_cxx.so)
    list(APPEND ALL_LIBRARIES ${BerkeleyDB_LIBS}/libdb_cxx.so)
elseif(EXISTS ${BerkeleyDB_LIBS}/libdb_cxx.dylib)
    list(APPEND ALL_LIBRARIES ${BerkeleyDB_LIBS}/libdb_cxx.dylib)
elseif(EXISTS ${BerkeleyDB_LIBS}/libdb_cxx.dll)
    list(APPEND ALL_LIBRARIES ${BerkeleyDB_LIBS}/libdb_cxx.dll)
else()
   message(FATAL_ERROR "Unable to find libdb_cxx library in ${BerkeleyDB_LIBS} directory. Try to specify either library path via BerkeleyDB_LIBS or berkeley db root path via BerkeleyDB_ROOT variable.")
endif()

list(APPEND ALL_DEFINITIONS QT_GUI USE_QRCODE)

if(USE_LEVELDB)
    # Disable useless targets
    option(LEVELDB_BUILD_TESTS "Build LevelDB's unit tests" OFF)
    option(LEVELDB_BUILD_BENCHMARKS "Build LevelDB's benchmarks" OFF)
    option(LEVELDB_INSTALL "Install LevelDB's header and library" OFF)

    add_subdirectory(src/additional/leveldb)
    list(APPEND ALL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/txdb-leveldb.cpp)
    list(APPEND ALL_LIBRARIES leveldb)
    list(APPEND ALL_DEFINITIONS USE_LEVELDB)
else()
    list(APPEND ALL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/txdb-bdb.cpp)
endif()

if (USE_ASM)
    # Assembler implementation
    set(asm_sources 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/crypto/scrypt/asm/scrypt-arm.S
        ${CMAKE_CURRENT_SOURCE_DIR}/src/crypto/scrypt/asm/scrypt-x86.S
        ${CMAKE_CURRENT_SOURCE_DIR}/src/crypto/scrypt/asm/scrypt-x86_64.S
    )

    list(APPEND ALL_SOURCES ${generic_sources} ${asm_sources})
    list(APPEND ALL_DEFINITIONS USE_ASM)
elseif (USE_SSE2)
    list( APPEND ALL_SOURCES ${generic_sources} ${CMAKE_CURRENT_SOURCE_DIR}/src/crypto/scrypt/intrin/scrypt-sse2.cpp )
    list(APPEND ALL_DEFINITIONS USE_SSE2)
else()
    list( APPEND ALL_SOURCES ${generic_sources} ${CMAKE_CURRENT_SOURCE_DIR}/src/crypto/scrypt/generic/scrypt-generic.cpp )
endif()

# Generate build info header
execute_process (
    COMMAND sh -c "${CMAKE_CURRENT_SOURCE_DIR}/share/genbuild.sh ${CMAKE_CURRENT_SOURCE_DIR}/src/build.h"
    OUTPUT_VARIABLE outVar
)
list(APPEND ALL_DEFINITIONS HAVE_BUILD_INFO)

add_executable(novacoin-qt ${ALL_SOURCES})
target_include_directories(novacoin-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/json ${BerkeleyDB_INC} ${CMAKE_CURRENT_SOURCE_DIR}/src/additional/leveldb/helpers ${Boost_INCLUDE_DIRS})
target_link_libraries(novacoin-qt ${ALL_LIBRARIES})

set_property(TARGET novacoin-qt PROPERTY CXX_STANDARD 17)
set_property(TARGET novacoin-qt PROPERTY CXX_STANDARD_REQUIRED TRUE)
set_property(TARGET novacoin-qt PROPERTY COMPILE_DEFINITIONS ${ALL_DEFINITIONS})
set_property(TARGET novacoin-qt PROPERTY CMAKE_WARN_DEPRECATED FALSE)
