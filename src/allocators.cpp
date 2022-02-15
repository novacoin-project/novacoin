// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "allocators.h"

#ifdef WIN32
#if (_WIN32_WINNT != _WIN32_WINNT_WIN7)
#define _WIN32_WINNT 0x601
#endif
#define WIN32_LEAN_AND_MEAN 1
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
// This is used to attempt to keep keying material out of swap
// Note that VirtualLock does not provide this as a guarantee on Windows,
// but, in practice, memory that has been VirtualLock'd almost never gets written to
// the pagefile except in rare circumstances where memory is extremely low.
#else
#include <sys/mman.h>
#include <climits> // for PAGESIZE
#include <unistd.h> // for sysconf
#endif

LockedPageManager* LockedPageManager::_instance = nullptr;
std::once_flag LockedPageManager::init_flag{};

/** Determine system page size in bytes */
static inline size_t GetSystemPageSize()
{
    size_t page_size;
#if defined(WIN32)
    SYSTEM_INFO sSysInfo;
    GetSystemInfo(&sSysInfo);
    page_size = sSysInfo.dwPageSize;
#elif defined(PAGESIZE) // defined in limits.h
    page_size = PAGESIZE;
#else // assume some POSIX OS
    page_size = sysconf(_SC_PAGESIZE);
#endif
    return page_size;
}

bool MemoryPageLocker::Lock(const void *addr, size_t len)
{
#ifdef WIN32
    return VirtualLock(const_cast<void*>(addr), len) != 0;
#else
    return mlock(addr, len) == 0;
#endif
}

bool MemoryPageLocker::Unlock(const void *addr, size_t len)
{
#ifdef WIN32
    return VirtualUnlock(const_cast<void*>(addr), len) != 0;
#else
    return munlock(addr, len) == 0;
#endif
}

LockedPageManager::LockedPageManager(): LockedPageManagerBase<MemoryPageLocker>(GetSystemPageSize())
{
}
