// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "uint256.h"

#include <cassert>
#include <cstring>


//////////////////////////////////////////////////////////////////////////////
//
// uint160
//

uint160::uint160()
{
    for (int i = 0; i < WIDTH; i++)
        pn[i] = 0;
}

uint160::uint160(const basetype& b)
{
    for (int i = 0; i < WIDTH; i++)
        pn[i] = b.pn[i];
}

uint160& uint160::operator=(const basetype& b)
{
    for (int i = 0; i < WIDTH; i++)
        pn[i] = b.pn[i];
    return *this;
}

uint160::uint160(uint64_t b)
{
    pn[0] = (uint32_t)b;
    pn[1] = (uint32_t)(b >> 32);
    for (int i = 2; i < WIDTH; i++)
        pn[i] = 0;
}

uint160& uint160::operator=(uint64_t b)
{
    pn[0] = (uint32_t)b;
    pn[1] = (uint32_t)(b >> 32);
    for (int i = 2; i < WIDTH; i++)
        pn[i] = 0;
    return *this;
}

uint160::uint160(const std::string& str)
{
    SetHex(str);
}

uint160::uint160(const std::vector<unsigned char>& vch)
{
    if (vch.size() == sizeof(pn))
        memcpy(pn, &vch[0], sizeof(pn));
    else
        *this = 0;
}

//////////////////////////////////////////////////////////////////////////////
//
// uint256
//

uint256::uint256()
{
    for (int i = 0; i < WIDTH; i++)
        pn[i] = 0;
}

uint256::uint256(const basetype& b)
{
    for (int i = 0; i < WIDTH; i++)
        pn[i] = b.pn[i];
}

uint256& uint256::operator=(const basetype& b)
{
    for (int i = 0; i < WIDTH; i++)
        pn[i] = b.pn[i];
    return *this;
}

uint256::uint256(uint64_t b)
{
    pn[0] = (uint32_t)b;
    pn[1] = (uint32_t)(b >> 32);
    for (int i = 2; i < WIDTH; i++)
        pn[i] = 0;
}

uint256& uint256::operator=(uint64_t b)
{
    pn[0] = (uint32_t)b;
    pn[1] = (uint32_t)(b >> 32);
    for (int i = 2; i < WIDTH; i++)
        pn[i] = 0;
    return *this;
}

uint256& uint256::SetCompact(uint32_t nCompact, bool *pfNegative, bool *pfOverflow)
{
    int nSize = nCompact >> 24;
    uint32_t nWord = nCompact & 0x007fffff;
    if (nSize <= 3) {
        nWord >>= 8*(3-nSize);
        *this = nWord;
    } else {
        *this = nWord;
        *this <<= 8*(nSize-3);
    }
    if (pfNegative)
        *pfNegative = nWord != 0 && (nCompact & 0x00800000) != 0;
    if (pfOverflow)
        *pfOverflow = nWord != 0 && ((nSize > 34) ||
                                 (nWord > 0xff && nSize > 33) ||
                                 (nWord > 0xffff && nSize > 32));
    return *this;
}

uint32_t uint256::GetCompact(bool fNegative) const
{
    int nSize = (bits() + 7) / 8;
    uint32_t nCompact = 0;
    if (nSize <= 3) {
        nCompact = Get64(0) << 8*(3-nSize);
    } else {
        uint256 n = *this;
        uint256 bn = n >> 8*(nSize-3);
        nCompact = bn.Get64(0);
    }
    // The 0x00800000 bit denotes the sign.
    // Thus, if it is already set, divide the mantissa by 256 and increase the exponent.
    if (nCompact & 0x00800000) {
        nCompact >>= 8;
        nSize++;
    }
    assert((nCompact & ~0x007fffff) == 0);
    assert(nSize < 256);
    nCompact |= nSize << 24;
    nCompact |= (fNegative && (nCompact & 0x007fffff) ? 0x00800000 : 0);
    return nCompact;
}

uint256& uint256::operator*=(uint32_t b32)
{
    uint64_t carry = 0;
    for (int i = 0; i < WIDTH; i++) {
        uint64_t n = carry + (uint64_t)b32 * pn[i];
        pn[i] = n & 0xffffffff;
        carry = n >> 32;
    }
    return *this;
}

uint256& uint256::operator*=(const uint256& b)
{
    uint256 a;
    for (int j = 0; j < WIDTH; j++) {
        uint64_t carry = 0;
        for (int i = 0; i + j < WIDTH; i++) {
            uint64_t n = carry + pn[i + j] + (uint64_t)a.pn[j] * b.pn[i];
            pn[i + j] = n & 0xffffffff;
            carry = n >> 32;
        }
    }
    *this = a;
    return *this;
}

uint256& uint256::operator/=(const uint256& b)
{
    uint256 div = b;     // make a copy, so we can shift.
    uint256 num = *this; // make a copy, so we can subtract.
    *this = 0;                   // the quotient.
    int num_bits = num.bits();
    int div_bits = div.bits();
    if (div_bits == 0)
        throw uint256_error("Division by zero");
    if (div_bits > num_bits) // the result is certainly 0.
        return *this;
    int shift = num_bits - div_bits;
    div <<= shift; // shift so that div and num align.
    while (shift >= 0) {
        if (num >= div) {
            num -= div;
            pn[shift / 32] |= (1 << (shift & 31)); // set a bit of the result.
        }
        div >>= 1; // shift back.
        shift--;
    }
    // num now contains the remainder of the division.
    return *this;
}

uint256::uint256(const std::string& str)
{
    SetHex(str);
}

uint256::uint256(const std::vector<unsigned char>& vch)
{
    if (vch.size() == sizeof(pn))
        memcpy(pn, &vch[0], sizeof(pn));
    else
        *this = 0;
}
