#include "bignum.h"
#include "uint256.h"


CAutoBN_CTX::CAutoBN_CTX() {
    pctx = BN_CTX_new();
    if (pctx == nullptr)
        throw bignum_error("CAutoBN_CTX : BN_CTX_new() returned NULL");
}

CAutoBN_CTX::~CAutoBN_CTX() {
    if (pctx != nullptr)
        BN_CTX_free(pctx);
}

CBigNum::CBigNum() {
    bn = BN_new();
}

CBigNum::CBigNum(const CBigNum &b) {
    BIGNUM *dup = BN_dup(b.bn);
    if (!dup) {
        throw bignum_error("CBigNum::CBigNum(const CBigNum&) : BN_dup failed");
    }
    bn = dup;
}

CBigNum &CBigNum::operator=(const CBigNum &b) {
    BIGNUM *dup = BN_dup(b.bn);
    if (!dup) {
        throw bignum_error("CBigNum::operator= : BN_dup failed");
    }
    bn = dup;
    return (*this);
}

CBigNum::CBigNum(const BIGNUM *bnp) {
    BIGNUM *dup = BN_dup(bnp);
    if (!dup) {
        throw bignum_error("CBigNum::CBigNum(const BIGNUM*) : BN_dup failed");
    }
    bn = dup;
}

CBigNum::~CBigNum() {
    BN_clear_free(bn);
}

CBigNum::CBigNum(uint256 n) { bn = BN_new(); setuint256(n); }

void CBigNum::setuint32(uint32_t n) {
    if (!BN_set_word(bn, n))
        throw bignum_error("CBigNum conversion from uint32_t : BN_set_word failed");
}

uint32_t CBigNum::getuint32() const {
    return BN_get_word(bn);
}

int32_t CBigNum::getint32() const {
    uint64_t n = BN_get_word(bn);
    if (!BN_is_negative(bn))
        return (n > (uint64_t)std::numeric_limits<int32_t>::max() ? std::numeric_limits<int32_t>::max() : (int32_t)n);
    else
        return (n > (uint64_t)std::numeric_limits<int32_t>::max() ? std::numeric_limits<int32_t>::min() : -(int32_t)n);
}

void CBigNum::setint64(int64_t sn) {
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

uint64_t CBigNum::getuint64() {
    size_t nSize = BN_bn2mpi(bn, nullptr);
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

void CBigNum::setuint64(uint64_t n) {
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

void CBigNum::setuint160(uint160 n) {
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

uint160 CBigNum::getuint160() const {
    unsigned int nSize = BN_bn2mpi(bn, nullptr);
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

void CBigNum::setuint256(uint256 n) {
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

uint256 CBigNum::getuint256() const {
    unsigned int nSize = BN_bn2mpi(bn, nullptr);
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

void CBigNum::setBytes(const std::vector<uint8_t> &vchBytes) {
    BN_bin2bn(&vchBytes[0], (int) vchBytes.size(), bn);
}

std::vector<uint8_t> CBigNum::getBytes() const {
    int nBytes = BN_num_bytes(bn);

    std::vector<uint8_t> vchBytes(nBytes);

    int n = BN_bn2bin(bn, &vchBytes[0]);
    if (n != nBytes) {
        throw bignum_error("CBigNum::getBytes : BN_bn2bin failed");
    }

    return vchBytes;
}

void CBigNum::setvch(const std::vector<uint8_t> &vch) {
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

std::vector<uint8_t> CBigNum::getvch() const {
    unsigned int nSize = BN_bn2mpi(bn, nullptr);
    if (nSize <= 4)
        return {};
    std::vector<uint8_t> vch(nSize);
    BN_bn2mpi(bn, &vch[0]);
    vch.erase(vch.begin(), vch.begin() + 4);
    std::reverse(vch.begin(), vch.end());
    return vch;
}

CBigNum &CBigNum::SetCompact(uint32_t nCompact) {
    uint32_t nSize = nCompact >> 24;
    std::vector<uint8_t> vch(4 + nSize);
    vch[3] = nSize;
    if (nSize >= 1) vch[4] = (nCompact >> 16) & 0xff;
    if (nSize >= 2) vch[5] = (nCompact >> 8) & 0xff;
    if (nSize >= 3) vch[6] = (nCompact >> 0) & 0xff;
    BN_mpi2bn(&vch[0], (int) vch.size(), bn);
    return *this;
}

uint32_t CBigNum::GetCompact() const {
    uint32_t nSize = BN_bn2mpi(bn, nullptr);
    std::vector<uint8_t> vch(nSize);
    nSize -= 4;
    BN_bn2mpi(bn, &vch[0]);
    uint32_t nCompact = nSize << 24;
    if (nSize >= 1) nCompact |= (vch[4] << 16);
    if (nSize >= 2) nCompact |= (vch[5] << 8);
    if (nSize >= 3) nCompact |= (vch[6] << 0);
    return nCompact;
}

void CBigNum::SetHex(const std::string &str) {
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

std::string CBigNum::ToString(int nBase) const {
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

bool CBigNum::operator!() const {
    return BN_is_zero(bn);
}

CBigNum &CBigNum::operator+=(const CBigNum &b) {
    if (!BN_add(bn, bn, b.bn))
        throw bignum_error("CBigNum::operator+= : BN_add failed");
    return *this;
}

CBigNum &CBigNum::operator-=(const CBigNum &b) {
    *this = *this - b;
    return *this;
}

CBigNum &CBigNum::operator*=(const CBigNum &b) {
    CAutoBN_CTX pctx;
    if (!BN_mul(bn, bn, b.bn, pctx))
        throw bignum_error("CBigNum::operator*= : BN_mul failed");
    return *this;
}

CBigNum &CBigNum::operator/=(const CBigNum &b) {
    *this = *this / b;
    return *this;
}

CBigNum &CBigNum::operator%=(const CBigNum &b) {
    *this = *this % b;
    return *this;
}

CBigNum &CBigNum::operator<<=(unsigned int shift) {
    if (!BN_lshift(bn, bn, shift))
        throw bignum_error("CBigNum:operator<<= : BN_lshift failed");
    return *this;
}

CBigNum &CBigNum::operator>>=(unsigned int shift) {
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

CBigNum &CBigNum::operator++() {
    // prefix operator
    if (!BN_add(bn, bn, BN_value_one()))
        throw bignum_error("CBigNum::operator++ : BN_add failed");
    return *this;
}

CBigNum &CBigNum::operator--() {
    // prefix operator
    CBigNum r;
    if (!BN_sub(r.bn, bn, BN_value_one()))
        throw bignum_error("CBigNum::operator-- : BN_sub failed");
    *this = r;
    return *this;
}

const CBigNum CBigNum::operator--(int) {
    // postfix operator
    const CBigNum ret = *this;
    --(*this);
    return ret;
}

const CBigNum CBigNum::operator++(int) {
    // postfix operator
    const CBigNum ret = *this;
    ++(*this);
    return ret;
}
