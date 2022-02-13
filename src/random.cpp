// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "random.h"
#include "uint256.h"
#include "util.h" // for printf()

#ifdef WIN32
#include "compat.h" // for Windows API
#endif

#ifndef WIN32
#include <sys/time.h>
#endif

#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <cstring>


static inline int64_t GetPerformanceCounter()
{
    int64_t nCounter = 0;
#ifdef WIN32
    QueryPerformanceCounter((LARGE_INTEGER*)&nCounter);
#else
    timeval t{};
    gettimeofday(&t, nullptr);
    nCounter = (int64_t) t.tv_sec * 1000000 + t.tv_usec;
#endif
    return nCounter;
}

void RandAddSeed()
{
    // Seed with CPU performance counter
    int64_t nCounter = GetPerformanceCounter();
    RAND_add(&nCounter, sizeof(nCounter), 1.5);
    memset(&nCounter, 0, sizeof(nCounter));
}

void RandAddSeedPerfmon()
{
    RandAddSeed();

#ifdef WIN32
    // This can take up to 2 seconds, so only do it every 10 minutes
    static int64_t nLastPerfmon;
    if (GetTime() < nLastPerfmon + 10 * 60)
        return;
    nLastPerfmon = GetTime();

    // Don't need this on Linux, OpenSSL automatically uses /dev/urandom
    // Seed with the entire set of perfmon data
    unsigned char pdata[250000];
    memset(pdata, 0, sizeof(pdata));
    unsigned long nSize = sizeof(pdata);
    long ret = RegQueryValueExA(HKEY_PERFORMANCE_DATA, "Global", NULL, NULL, pdata, &nSize);
    RegCloseKey(HKEY_PERFORMANCE_DATA);
    if (ret == ERROR_SUCCESS)
    {
        RAND_add(pdata, nSize, nSize/100.0);
        OPENSSL_cleanse(pdata, nSize);
        printf("RandAddSeed() %lu bytes\n", nSize);
    }
#endif
}

void GetRandBytes(unsigned char *buf, int num)
{
    if (RAND_bytes(buf, num) != 1) {
        printf("%s: OpenSSL RAND_bytes() failed with error: %s\n", __func__, ERR_error_string(ERR_get_error(), nullptr));
        assert(false);
    }
}

uint64_t GetRand(uint64_t nMax)
{
    if (nMax == 0)
        return 0;

    // The range of the random source must be a multiple of the modulus
    // to give every possible output value an equal possibility
    uint64_t nRange = (std::numeric_limits<uint64_t>::max() / nMax) * nMax;
    uint64_t nRand = 0;
    do
        RAND_bytes((unsigned char*)&nRand, sizeof(nRand));
    while (nRand >= nRange);
    return (nRand % nMax);
}

int GetRandInt(int nMax)
{
    return static_cast<int>(GetRand(nMax));
}

uint256 GetRandHash()
{
    uint256 hash;
    RAND_bytes(hash.begin(), hash.size());
    return hash;
}
