cmake_minimum_required(VERSION 3.4.1)

##
## mkdir build && cd build
##
## cmake -DBerkeleyDB_ROOT:STRING=/opt/homebrew/Cellar/berkeley-db@4/4.8.30 ..
## cmake -DUSE_ASM=1 ..
## cmake -DUSE_SSE2 ..
## cmake -DBerkeleyDB_INC:STRING=/usr/include -DBerkeleyDB_LIBS:STRING=/usr/lib/aarch64-linux-gnu -DUSE_SSE2 -DUSE_LEVELDB ..
##

project(novacoind VERSION 0.5.9 LANGUAGES C CXX)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set(CMAKE_C_COMPILER_LAUNCHER ccache)
    set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
endif(CCACHE_FOUND)

set (CMAKE_C_FLAGS "-flto -O3 -fPIC -Wno-deprecated -fstack-protector-all -Wstack-protector -D_FORTIFY_SOURCE=2")
set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fexceptions -frtti")

# Add path for custom modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/additional/cmake)

set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost REQUIRED COMPONENTS atomic chrono filesystem program_options system thread)
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
list(APPEND ALL_LIBRARIES ${BerkeleyDB_LIBS}/libdb_cxx.a ${Boost_LIBRARIES} ixwebsocket OpenSSL::Crypto)

if(USE_LEVELDB)
    add_subdirectory(additional/leveldb)
    list(APPEND ALL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/txdb-leveldb.cpp)
    list(APPEND ALL_LIBRARIES leveldb)
    list(APPEND ALL_DEFINITIONS USE_LEVELDB)
else()
    list(APPEND ALL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/txdb-bdb.cpp)
endif()

if (USE_ASM)
    # Assembler implementation
    set(asm_sources 
        ${CMAKE_CURRENT_SOURCE_DIR}/crypto/scrypt/asm/scrypt-arm.S
        ${CMAKE_CURRENT_SOURCE_DIR}/crypto/scrypt/asm/scrypt-x86.S
        ${CMAKE_CURRENT_SOURCE_DIR}/crypto/scrypt/asm/scrypt-x86_64.S
    )

    list(APPEND ALL_SOURCES ${generic_sources} ${asm_sources})
    list(APPEND ALL_DEFINITIONS USE_ASM)
elseif (USE_SSE2)
    list( APPEND ALL_SOURCES ${generic_sources} ${CMAKE_CURRENT_SOURCE_DIR}/crypto/scrypt/intrin/scrypt-sse2.cpp )
    list(APPEND ALL_DEFINITIONS USE_SSE2)
else()
    list( APPEND ALL_SOURCES ${generic_sources} ${CMAKE_CURRENT_SOURCE_DIR}/crypto/scrypt/generic/scrypt-generic.cpp )
endif()

add_executable(novacoind ${ALL_SOURCES})
target_include_directories(novacoind PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/json ${BerkeleyDB_INC} ${CMAKE_CURRENT_SOURCE_DIR}/additional/leveldb/helpers ${Boost_INCLUDE_DIRS})
target_link_libraries(novacoind ${ALL_LIBRARIES})

set_property(TARGET novacoind PROPERTY CXX_STANDARD 17)
set_property(TARGET novacoind PROPERTY CXX_STANDARD_REQUIRED TRUE)
set_property(TARGET novacoind PROPERTY COMPILE_DEFINITIONS ${ALL_DEFINITIONS})
set_property(TARGET novacoind PROPERTY CMAKE_WARN_DEPRECATED FALSE)
