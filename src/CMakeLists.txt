cmake_minimum_required(VERSION 3.4.1)

##
## mkdir build && cd build
##
## cmake -DBerkeleyDB_ROOT:STRING=/opt/homebrew/Cellar/berkeley-db@4/4.8.30 ..
## cmake -DBerkeleyDB_INC:STRING=/usr/include -DBerkeleyDB_LIBS:STRING=/usr/lib/aarch64-linux-gnu -DUSE_LEVELDB=1 ..
##

project(novacoind VERSION 0.5.9 LANGUAGES C CXX)

# Enforce sizeof(size_t) == 8 by default
if (NOT ALLOW_32BIT AND NOT CMAKE_SIZEOF_VOID_P MATCHES "8")
    message(FATAL_ERROR "Only 64-bit processors (x86_64, AArch64) are supported")
endif ()

# Force generic scrypt on 32 bit platforms
if (NOT CMAKE_SIZEOF_VOID_P MATCHES "8")
    set(USE_GENERIC_SCRYPT True)
endif()

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set(CMAKE_C_COMPILER_LAUNCHER ccache)
    set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
endif(CCACHE_FOUND)

set (CMAKE_C_FLAGS "-flto -O3 -fPIC -Wno-deprecated -fstack-protector-all -Wstack-protector -D_FORTIFY_SOURCE=2")
set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fexceptions -frtti")
set(CMAKE_CXX_EXTENSIONS OFF)

set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost REQUIRED COMPONENTS atomic chrono filesystem program_options system thread)
set(OPENSSL_USE_STATIC_LIBS ON)
find_package(OpenSSL REQUIRED)
add_subdirectory(additional/IXWebSocket)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/addrman.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/alert.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/base58.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bitcoinrpc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/checkpoints.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/crypter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/db.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/init.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ipcollector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/irc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel_worker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kernelrecord.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/key.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/keystore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/miner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/net.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/netbase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/noui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ntp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rpcblockchain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rpcdump.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rpcmining.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rpcnet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rpcrawtransaction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rpcwallet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/script.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stun.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sync.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wallet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/walletdb.cpp
)

list(APPEND ALL_SOURCES ${generic_sources})
list(APPEND ALL_LIBRARIES ${Boost_LIBRARIES} ixwebsocket OpenSSL::Crypto)

# Windows-specific libraries
if (WIN32)
   list(APPEND ALL_LIBRARIES kernel32 user32 gdi32 comdlg32 winspool winmm shell32 comctl32 ole32 oleaut32 uuid rpcrt4 advapi32 ws2_32 mswsock shlwapi)
endif()

# Some systems need rt library
if (NOT WIN32 AND NOT APPLE)
    list(APPEND ALL_LIBRARIES rt)
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
    set(BerkeleyDB_LIBS ${BerkeleyDB_ROOT}/lib/x86_64-linux-gnu)
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
endif()

if(USE_LEVELDB)
    # Disable useless targets
    option(LEVELDB_BUILD_TESTS "Build LevelDB's unit tests" OFF)
    option(LEVELDB_BUILD_BENCHMARKS "Build LevelDB's benchmarks" OFF)
    option(LEVELDB_INSTALL "Install LevelDB's header and library" OFF)

    add_subdirectory(additional/leveldb)
    list(APPEND ALL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/txdb-leveldb.cpp)
    list(APPEND ALL_LIBRARIES leveldb)
    list(APPEND ALL_DEFINITIONS USE_LEVELDB)
else()
    list(APPEND ALL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/txdb-bdb.cpp)
endif()

if (NOT USE_GENERIC_SCRYPT)
    list( APPEND ALL_SOURCES ${generic_sources} ${CMAKE_CURRENT_SOURCE_DIR}/crypto/scrypt/intrin/scrypt-intrin.cpp )
    list(APPEND ALL_DEFINITIONS USE_INTRIN)
else()
    list( APPEND ALL_SOURCES ${generic_sources} ${CMAKE_CURRENT_SOURCE_DIR}/crypto/scrypt/generic/scrypt-generic.cpp )
endif()

# Generate build info header
execute_process (
    COMMAND sh -c "${CMAKE_CURRENT_SOURCE_DIR}/../share/genbuild.sh ${CMAKE_CURRENT_SOURCE_DIR}/build.h"
    OUTPUT_VARIABLE outVar
)
list(APPEND ALL_DEFINITIONS HAVE_BUILD_INFO)

add_executable(novacoind ${ALL_SOURCES})
target_include_directories(novacoind PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/json ${BerkeleyDB_INC} ${CMAKE_CURRENT_SOURCE_DIR}/additional/sse2neon ${CMAKE_CURRENT_SOURCE_DIR}/additional/leveldb/helpers ${Boost_INCLUDE_DIRS})
target_link_libraries(novacoind ${ALL_LIBRARIES})

target_compile_features(novacoind PUBLIC cxx_std_17)
set_property(TARGET novacoind PROPERTY CXX_STANDARD 17)
set_property(TARGET novacoind PROPERTY CXX_STANDARD_REQUIRED TRUE)
set_property(TARGET novacoind PROPERTY COMPILE_DEFINITIONS ${ALL_DEFINITIONS})
set_property(TARGET novacoind PROPERTY CMAKE_WARN_DEPRECATED FALSE)
