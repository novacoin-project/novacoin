// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_BIGNUM_H
#define BITCOIN_BIGNUM_H


#include "serialize.h"
#include "version.h"

#include <openssl/bn.h>

#include <stdexcept>
#include <vector>
#include <algorithm>

class uint160;
class uint256;

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
    CAutoBN_CTX();
    ~CAutoBN_CTX();

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
    CBigNum();
    CBigNum(const CBigNum& b);
    CBigNum& operator=(const CBigNum& b);
    CBigNum(const BIGNUM *bnp);

    ~CBigNum();

    CBigNum(bool n)     { bn = BN_new(); setuint32(n); }

    CBigNum(int8_t  n)  { bn = BN_new(); if (n >= 0) setuint32(n); else setint64(n); }
    CBigNum(int16_t n)  { bn = BN_new(); if (n >= 0) setuint32(n); else setint64(n); }
    CBigNum(int32_t n)  { bn = BN_new(); if (n >= 0) setuint32(n); else setint64(n); }
    CBigNum(int64_t n)  { bn = BN_new(); if (n >= 0) setuint64(n); else setint64(n); }

    CBigNum(uint8_t  n) { bn = BN_new(); setuint32(n); }
    CBigNum(uint16_t n) { bn = BN_new(); setuint32(n); }
    CBigNum(uint32_t n) { bn = BN_new(); setuint32(n); }
    CBigNum(uint64_t n) { bn = BN_new(); setuint64(n); }

    explicit CBigNum(uint256 n);
    explicit CBigNum(const std::vector<uint8_t>& vch) { bn = BN_new(); setvch(vch); }

    void setuint32(uint32_t n);
    uint32_t getuint32() const;
    int32_t getint32() const;
    void setint64(int64_t sn);
    uint64_t getuint64();

    //supress msvc C4127: conditional expression is constant
    inline bool check(bool value) {return value;}

    void setuint64(uint64_t n);
    void setuint160(uint160 n);
    uint160 getuint160() const;
    void setuint256(uint256 n);
    uint256 getuint256() const;
    void setBytes(const std::vector<uint8_t>& vchBytes);
    std::vector<uint8_t> getBytes() const;
    void setvch(const std::vector<uint8_t>& vch);
    std::vector<uint8_t> getvch() const;
    CBigNum& SetCompact(uint32_t nCompact);
    uint32_t GetCompact() const;
    void SetHex(const std::string& str);
    std::string ToString(int nBase=10) const;
    std::string GetHex() const { return ToString(16); }
    BIGNUM* get() const { return BN_dup(bn); }

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

    bool operator!() const;

    CBigNum& operator+=(const CBigNum& b);
    CBigNum& operator-=(const CBigNum& b);
    CBigNum& operator*=(const CBigNum& b);
    CBigNum& operator/=(const CBigNum& b);
    CBigNum& operator%=(const CBigNum& b);
    CBigNum& operator<<=(unsigned int shift);
    CBigNum& operator>>=(unsigned int shift);
    CBigNum& operator++();
    const CBigNum operator++(int);
    CBigNum& operator--();
    const CBigNum operator--(int);

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
