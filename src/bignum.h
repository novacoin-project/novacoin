// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_BIGNUM_H
#define BITCOIN_BIGNUM_H


#include "serialize.h"
#include "uint256.h"

#include <openssl/bn.h>

#include <stdexcept>
#include <vector>
#include <algorithm>

/** Errors thrown by the bignum class */
class bignum_error : public std::runtime_error
{
public:
    explicit bignum_error(const std::string& str) : std::runtime_error(str) {}
};


/** RAII encapsulated BN_CTX (OpenSSL bignum context) */
class CAutoBN_CTX
{
protected:
    BN_CTX* pctx;
    BN_CTX* operator=(BN_CTX* pnew) { return pctx = pnew; }

public:
    CAutoBN_CTX()
    {
        pctx = BN_CTX_new();
        if (pctx == NULL)
            throw bignum_error("CAutoBN_CTX : BN_CTX_new() returned NULL");
    }

    ~CAutoBN_CTX()
    {
        if (pctx != NULL)
            BN_CTX_free(pctx);
    }

    operator BN_CTX*() { return pctx; }
    BN_CTX& operator*() { return *pctx; }
    BN_CTX** operator&() { return &pctx; }
    bool operator!() { return (pctx == NULL); }
};


/** C++ wrapper for BIGNUM (OpenSSL bignum) */
class CBigNum
{
private:
    BIGNUM* bn;
public:
    CBigNum()
    {
        bn = BN_new();
    }

    CBigNum(const CBigNum& b)
    {
        BIGNUM *dup = BN_dup(b.bn);
        if (!dup)
        {
            throw bignum_error("CBigNum::CBigNum(const CBigNum&) : BN_dup failed");
        }
        bn = dup;
    }

    CBigNum& operator=(const CBigNum& b)
    {
        BIGNUM *dup = BN_dup(b.bn);
        if (!dup)
        {
            throw bignum_error("CBigNum::operator= : BN_dup failed");
        }
        bn = dup;
        return (*this);
    }

    CBigNum(const BIGNUM *bnp) {
        BIGNUM *dup = BN_dup(bnp);
        if (!dup)
        {
            throw bignum_error("CBigNum::CBigNum(const BIGNUM*) : BN_dup failed");
        }
        bn = dup;
    }

    ~CBigNum()
    {
        BN_clear_free(bn);
    }

    CBigNum(bool n)     { bn = BN_new(); setuint32(n); }

    CBigNum(int8_t  n)  { bn = BN_new(); if (n >= 0) setuint32(n); else setint64(n); }
    CBigNum(int16_t n)  { bn = BN_new(); if (n >= 0) setuint32(n); else setint64(n); }
    CBigNum(int32_t n)  { bn = BN_new(); if (n >= 0) setuint32(n); else setint64(n); }
    CBigNum(int64_t n)  { bn = BN_new(); if (n >= 0) setuint64(n); else setint64(n); }

    CBigNum(uint8_t  n) { bn = BN_new(); setuint32(n); }
    CBigNum(uint16_t n) { bn = BN_new(); setuint32(n); }
    CBigNum(uint32_t n) { bn = BN_new(); setuint32(n); }
    CBigNum(uint64_t n) { bn = BN_new(); setuint64(n); }

    explicit CBigNum(uint256 n) { bn = BN_new(); setuint256(n); }
    explicit CBigNum(const std::vector<uint8_t>& vch)
    {
        bn = BN_new();
        setvch(vch);
    }

    void setuint32(uint32_t n)
    {
        if (!BN_set_word(bn, n))
            throw bignum_error("CBigNum conversion from uint32_t : BN_set_word failed");
    }

    uint32_t getuint32() const
    {
        return BN_get_word(bn);
    }

    int32_t getint32() const
    {
        uint64_t n = BN_get_word(bn);
        if (!BN_is_negative(bn))
            return (n > (uint64_t)std::numeric_limits<int32_t>::max() ? std::numeric_limits<int32_t>::max() : (int32_t)n);
        else
            return (n > (uint64_t)std::numeric_limits<int32_t>::max() ? std::numeric_limits<int32_t>::min() : -(int32_t)n);
    }

    void setint64(int64_t sn)
    {
        uint8_t pch[sizeof(sn) + 6];
        uint8_t* p = pch + 4;
        bool fNegative;
        uint64_t n;

        if (sn < (int64_t)0)
        {
            // Since the minimum signed integer cannot be represented as positive so long as its type is signed, and it's not well-defined what happens if you make it unsigned before negating it, we instead increment the negative integer by 1, convert it, then increment the (now positive) unsigned integer by 1 to compensate
            n = -(sn + 1);
            ++n;
            fNegative = true;
        } else {
            n = sn;
            fNegative = false;
        }

        bool fLeadingZeroes = true;
        for (int i = 0; i < 8; i++)
        {
            uint8_t c = (n >> 56) & 0xff;
            n <<= 8;
            if (fLeadingZeroes)
            {
                if (c == 0)
                    continue;
                if (c & 0x80)
                    *p++ = (fNegative ? 0x80 : 0);
                else if (fNegative)
                    c |= 0x80;
                fLeadingZeroes = false;
            }
            *p++ = c;
        }
        uint32_t nSize = (uint32_t) (p - (pch + 4));
        pch[0] = (nSize >> 24) & 0xff;
        pch[1] = (nSize >> 16) & 0xff;
        pch[2] = (nSize >> 8) & 0xff;
        pch[3] = (nSize) & 0xff;
        BN_mpi2bn(pch, (int)(p - pch), bn);
    }

    uint64_t getuint64()
    {
        size_t nSize = BN_bn2mpi(bn, NULL);
        if (nSize < 4)
            return 0;
        std::vector<uint8_t> vch(nSize);
        BN_bn2mpi(bn, &vch[0]);
        if (vch.size() > 4)
            vch[4] &= 0x7f;
        uint64_t n = 0;
        for (size_t i = 0, j = vch.size()-1; i < sizeof(n) && j >= 4; i++, j--)
            ((uint8_t*)&n)[i] = vch[j];
        return n;
    }

    //supress msvc C4127: conditional expression is constant
    inline bool check(bool value) {return value;}

    void setuint64(uint64_t n)
    {
        // Use BN_set_word if word size is sufficient for uint64_t
        if (check(sizeof(n) <= sizeof(BN_ULONG)))
        {
            if (!BN_set_word(bn, (BN_ULONG)n))
                throw bignum_error("CBigNum conversion from uint64_t : BN_set_word failed");
            return;
        }

        uint8_t pch[sizeof(n) + 6];
        uint8_t* p = pch + 4;
        bool fLeadingZeroes = true;
        for (int i = 0; i < 8; i++)
        {
            uint8_t c = (n >> 56) & 0xff;
            n <<= 8;
            if (fLeadingZeroes)
            {
                if (c == 0)
                    continue;
                if (c & 0x80)
                    *p++ = 0;
                fLeadingZeroes = false;
            }
            *p++ = c;
        }
        uint32_t nSize = (uint32_t) (p - (pch + 4));
        pch[0] = (nSize >> 24) & 0xff;
        pch[1] = (nSize >> 16) & 0xff;
        pch[2] = (nSize >> 8) & 0xff;
        pch[3] = (nSize) & 0xff;
        BN_mpi2bn(pch, (int)(p - pch), bn);
    }

    void setuint160(uint160 n)
    {
        uint8_t pch[sizeof(n) + 6];
        uint8_t* p = pch + 4;
        bool fLeadingZeroes = true;
        uint8_t* pbegin = (uint8_t*)&n;
        uint8_t* psrc = pbegin + sizeof(n);
        while (psrc != pbegin)
        {
            uint8_t c = *(--psrc);
            if (fLeadingZeroes)
            {
                if (c == 0)
                    continue;
                if (c & 0x80)
                    *p++ = 0;
                fLeadingZeroes = false;
            }
            *p++ = c;
        }
        uint32_t nSize = (uint32_t) (p - (pch + 4));
        pch[0] = (nSize >> 24) & 0xff;
        pch[1] = (nSize >> 16) & 0xff;
        pch[2] = (nSize >> 8) & 0xff;
        pch[3] = (nSize >> 0) & 0xff;
        BN_mpi2bn(pch, (int) (p - pch), bn);
    }

    uint160 getuint160() const
    {
        unsigned int nSize = BN_bn2mpi(bn, NULL);
        if (nSize < 4)
            return 0;
        std::vector<uint8_t> vch(nSize);
        BN_bn2mpi(bn, &vch[0]);
        if (vch.size() > 4)
            vch[4] &= 0x7f;
        uint160 n = 0;
        for (size_t i = 0, j = vch.size()-1; i < sizeof(n) && j >= 4; i++, j--)
            ((uint8_t*)&n)[i] = vch[j];
        return n;
    }

    void setuint256(uint256 n)
    {
        uint8_t pch[sizeof(n) + 6];
        uint8_t* p = pch + 4;
        bool fLeadingZeroes = true;
        uint8_t* pbegin = (uint8_t*)&n;
        uint8_t* psrc = pbegin + sizeof(n);
        while (psrc != pbegin)
        {
            uint8_t c = *(--psrc);
            if (fLeadingZeroes)
            {
                if (c == 0)
                    continue;
                if (c & 0x80)
                    *p++ = 0;
                fLeadingZeroes = false;
            }
            *p++ = c;
        }
        uint32_t nSize = (uint32_t) (p - (pch + 4));
        pch[0] = (nSize >> 24) & 0xff;
        pch[1] = (nSize >> 16) & 0xff;
        pch[2] = (nSize >> 8) & 0xff;
        pch[3] = (nSize >> 0) & 0xff;
        BN_mpi2bn(pch, (int) (p - pch), bn);
    }

    uint256 getuint256() const
    {
        unsigned int nSize = BN_bn2mpi(bn, NULL);
        if (nSize < 4)
            return 0;
        std::vector<uint8_t> vch(nSize);
        BN_bn2mpi(bn, &vch[0]);
        if (vch.size() > 4)
            vch[4] &= 0x7f;
        uint256 n = 0;
        for (size_t i = 0, j = vch.size()-1; i < sizeof(n) && j >= 4; i++, j--)
            ((uint8_t*)&n)[i] = vch[j];
        return n;
    }

    void setBytes(const std::vector<uint8_t>& vchBytes)
    {
        BN_bin2bn(&vchBytes[0], (int) vchBytes.size(), bn);
    }

    std::vector<uint8_t> getBytes() const
    {
        int nBytes = BN_num_bytes(bn);

        std::vector<uint8_t> vchBytes(nBytes);

        int n = BN_bn2bin(bn, &vchBytes[0]);
        if (n != nBytes) {
            throw bignum_error("CBigNum::getBytes : BN_bn2bin failed");
        }

        return vchBytes;
    }

    void setvch(const std::vector<uint8_t>& vch)
    {
        std::vector<uint8_t> vch2(vch.size() + 4);
        uint32_t nSize = (uint32_t) vch.size();
        // BIGNUM's byte stream format expects 4 bytes of
        // big endian size data info at the front
        vch2[0] = (nSize >> 24) & 0xff;
        vch2[1] = (nSize >> 16) & 0xff;
        vch2[2] = (nSize >> 8) & 0xff;
        vch2[3] = (nSize >> 0) & 0xff;
        // swap data to big endian
        std::reverse_copy(vch.begin(), vch.end(), vch2.begin() + 4);
        BN_mpi2bn(&vch2[0], (int) vch2.size(), bn);
    }

    std::vector<uint8_t> getvch() const
    {
        unsigned int nSize = BN_bn2mpi(bn, NULL);
        if (nSize <= 4)
            return std::vector<uint8_t>();
        std::vector<uint8_t> vch(nSize);
        BN_bn2mpi(bn, &vch[0]);
        vch.erase(vch.begin(), vch.begin() + 4);
        std::reverse(vch.begin(), vch.end());
        return vch;
    }

    CBigNum& SetCompact(uint32_t nCompact)
    {
        uint32_t nSize = nCompact >> 24;
        std::vector<uint8_t> vch(4 + nSize);
        vch[3] = nSize;
        if (nSize >= 1) vch[4] = (nCompact >> 16) & 0xff;
        if (nSize >= 2) vch[5] = (nCompact >> 8) & 0xff;
        if (nSize >= 3) vch[6] = (nCompact >> 0) & 0xff;
        BN_mpi2bn(&vch[0], (int) vch.size(), bn);
        return *this;
    }

    uint32_t GetCompact() const
    {
        uint32_t nSize = BN_bn2mpi(bn, NULL);
        std::vector<uint8_t> vch(nSize);
        nSize -= 4;
        BN_bn2mpi(bn, &vch[0]);
        uint32_t nCompact = nSize << 24;
        if (nSize >= 1) nCompact |= (vch[4] << 16);
        if (nSize >= 2) nCompact |= (vch[5] << 8);
        if (nSize >= 3) nCompact |= (vch[6] << 0);
        return nCompact;
    }

    void SetHex(const std::string& str)
    {
        // skip 0x
        const char* psz = str.c_str();
        while (isspace(*psz))
            psz++;
        bool fNegative = false;
        if (*psz == '-')
        {
            fNegative = true;
            psz++;
        }
        if (psz[0] == '0' && tolower(psz[1]) == 'x')
            psz += 2;
        while (isspace(*psz))
            psz++;

        // hex string to bignum
        static const signed char phexdigit[256] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0, 0,0xa,0xb,0xc,0xd,0xe,0xf,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0xa,0xb,0xc,0xd,0xe,0xf,0,0,0,0,0,0,0,0,0 };
        *this = 0;
        while (isxdigit(*psz))
        {
            *this <<= 4;
            int n = phexdigit[(uint8_t)*psz++];
            *this += n;
        }
        if (fNegative)
            *this = 0 - *this;
    }

    std::string ToString(int nBase=10) const
    {
        CAutoBN_CTX pctx;
        CBigNum bnBase = nBase;
        CBigNum bn0 = 0;
        std::string str;
        CBigNum bn = *this;
        BN_set_negative(bn.bn, false);
        CBigNum dv;
        CBigNum rem;
        if (BN_cmp(bn.bn, bn0.bn) == 0)
            return "0";
        while (BN_cmp(bn.bn, bn0.bn) > 0)
        {
            if (!BN_div(dv.bn, rem.bn, bn.bn, bnBase.bn, pctx))
                throw bignum_error("CBigNum::ToString() : BN_div failed");
            bn = dv;
            uint32_t c = rem.getuint32();
            str += "0123456789abcdef"[c];
        }
        if (BN_is_negative(bn.bn))
            str += "-";
        std::reverse(str.begin(), str.end());
        return str;
    }

    std::string GetHex() const
    {
        return ToString(16);
    }

    BIGNUM* get() const {
        return BN_dup(bn);
    }

    unsigned int GetSerializeSize(int nType=0, int nVersion=PROTOCOL_VERSION) const
    {
        return ::GetSerializeSize(getvch(), nType, nVersion);
    }

    template<typename Stream>
    void Serialize(Stream& s, int nType=0, int nVersion=PROTOCOL_VERSION) const
    {
        ::Serialize(s, getvch(), nType, nVersion);
    }

    template<typename Stream>
    void Unserialize(Stream& s, int nType=0, int nVersion=PROTOCOL_VERSION)
    {
        std::vector<uint8_t> vch;
        ::Unserialize(s, vch, nType, nVersion);
        setvch(vch);
    }

    bool operator!() const
    {
        return BN_is_zero(bn);
    }

    CBigNum& operator+=(const CBigNum& b)
    {
        if (!BN_add(bn, bn, b.bn))
            throw bignum_error("CBigNum::operator+= : BN_add failed");
        return *this;
    }

    CBigNum& operator-=(const CBigNum& b)
    {
        *this = *this - b;
        return *this;
    }

    CBigNum& operator*=(const CBigNum& b)
    {
        CAutoBN_CTX pctx;
        if (!BN_mul(bn, bn, b.bn, pctx))
            throw bignum_error("CBigNum::operator*= : BN_mul failed");
        return *this;
    }

    CBigNum& operator/=(const CBigNum& b)
    {
        *this = *this / b;
        return *this;
    }

    CBigNum& operator%=(const CBigNum& b)
    {
        *this = *this % b;
        return *this;
    }

    CBigNum& operator<<=(unsigned int shift)
    {
        if (!BN_lshift(bn, bn, shift))
            throw bignum_error("CBigNum:operator<<= : BN_lshift failed");
        return *this;
    }

    CBigNum& operator>>=(unsigned int shift)
    {
        // Note: BN_rshift segfaults on 64-bit if 2^shift is greater than the number
        //   if built on ubuntu 9.04 or 9.10, probably depends on version of OpenSSL
        CBigNum a = 1;
        a <<= shift;
        if (BN_cmp(a.bn, bn) > 0)
        {
            *this = 0;
            return *this;
        }

        if (!BN_rshift(bn, bn, shift))
            throw bignum_error("CBigNum:operator>>= : BN_rshift failed");
        return *this;
    }

    CBigNum& operator++()
    {
        // prefix operator
        if (!BN_add(bn, bn, BN_value_one()))
            throw bignum_error("CBigNum::operator++ : BN_add failed");
        return *this;
    }

    const CBigNum operator++(int)
    {
        // postfix operator
        const CBigNum ret = *this;
        ++(*this);
        return ret;
    }

    CBigNum& operator--()
    {
        // prefix operator
        CBigNum r;
        if (!BN_sub(r.bn, bn, BN_value_one()))
            throw bignum_error("CBigNum::operator-- : BN_sub failed");
        *this = r;
        return *this;
    }

    const CBigNum operator--(int)
    {
        // postfix operator
        const CBigNum ret = *this;
        --(*this);
        return ret;
    }

    friend inline const CBigNum operator-(const CBigNum& a, const CBigNum& b);
    friend inline const CBigNum operator/(const CBigNum& a, const CBigNum& b);
    friend inline const CBigNum operator%(const CBigNum& a, const CBigNum& b);
    friend inline const CBigNum operator*(const CBigNum& a, const CBigNum& b);
    friend inline const CBigNum operator+(const CBigNum& a, const CBigNum& b);
    friend inline const CBigNum operator*(const CBigNum& a);
    
    friend inline const CBigNum operator-(const CBigNum& a);
    friend inline const CBigNum operator<<(const CBigNum& a, unsigned int shift);
    
    friend inline bool operator==(const CBigNum& a, const CBigNum& b);
    friend inline bool operator!=(const CBigNum& a, const CBigNum& b);
    friend inline bool operator<=(const CBigNum& a, const CBigNum& b);
    friend inline bool operator>=(const CBigNum& a, const CBigNum& b);
    friend inline bool operator<(const CBigNum& a, const CBigNum& b);
    friend inline bool operator>(const CBigNum& a, const CBigNum& b);
    friend inline std::ostream& operator<<(std::ostream &strm, const CBigNum &b);
};


inline const CBigNum operator+(const CBigNum& a, const CBigNum& b)
{
    CBigNum r;
    if (!BN_add(r.bn, a.bn, b.bn))
        throw bignum_error("CBigNum::operator+ : BN_add failed");
    return r;
}

inline const CBigNum operator-(const CBigNum& a, const CBigNum& b)
{
    CBigNum r;
    if (!BN_sub(r.bn, a.bn, b.bn))
        throw bignum_error("CBigNum::operator- : BN_sub failed");
    return r;
}

inline const CBigNum operator-(const CBigNum& a)
{
    CBigNum r(a);
    BN_set_negative(r.bn, !BN_is_negative(r.bn));
    return r;
}

inline const CBigNum operator*(const CBigNum& a, const CBigNum& b)
{
    CAutoBN_CTX pctx;
    CBigNum r;
    if (!BN_mul(r.bn, a.bn, b.bn, pctx))
        throw bignum_error("CBigNum::operator* : BN_mul failed");
    return r;
}

inline const CBigNum operator/(const CBigNum& a, const CBigNum& b)
{
    CAutoBN_CTX pctx;
    CBigNum r;
    if (!BN_div(r.bn, NULL, a.bn, b.bn, pctx))
        throw bignum_error("CBigNum::operator/ : BN_div failed");
    return r;
}

inline const CBigNum operator%(const CBigNum& a, const CBigNum& b)
{
    CAutoBN_CTX pctx;
    CBigNum r;
    if (!BN_nnmod(r.bn, a.bn, b.bn, pctx))
        throw bignum_error("CBigNum::operator% : BN_div failed");
    return r;
}

inline const CBigNum operator<<(const CBigNum& a, unsigned int shift)
{
    CBigNum r;
    if (!BN_lshift(r.bn, a.bn, shift))
        throw bignum_error("CBigNum:operator<< : BN_lshift failed");
    return r;
}

inline const CBigNum operator>>(const CBigNum& a, unsigned int shift)
{
    CBigNum r = a;
    r >>= shift;
    return r;
}

inline bool operator==(const CBigNum& a, const CBigNum& b) { return (BN_cmp(a.bn, b.bn) == 0); }
inline bool operator!=(const CBigNum& a, const CBigNum& b) { return (BN_cmp(a.bn, b.bn) != 0); }
inline bool operator<=(const CBigNum& a, const CBigNum& b) { return (BN_cmp(a.bn, b.bn) <= 0); }
inline bool operator>=(const CBigNum& a, const CBigNum& b) { return (BN_cmp(a.bn, b.bn) >= 0); }
inline bool operator<(const CBigNum& a, const CBigNum& b)  { return (BN_cmp(a.bn, b.bn) < 0); }
inline bool operator>(const CBigNum& a, const CBigNum& b)  { return (BN_cmp(a.bn, b.bn) > 0); }

inline std::ostream& operator<<(std::ostream &strm, const CBigNum &b) { return strm << b.ToString(10); }

#endif
