// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


//
// Why base-58 instead of standard base-64 encoding?
// - Don't want 0OIl characters that look the same in some fonts and
//      could be used to create visually identical looking account numbers.
// - A string with non-alphanumeric characters is not as easily accepted as an account number.
// - E-mail usually won't line-break if there's no punctuation to break at.
// - Double-clicking selects the whole number as one word if it's all alphanumeric.
//

#include "base58.h"
#include "hash.h"

static const std::array<char, 58> digits = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
    'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};

static const std::array<signed char, 128> characterMap = {
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8,-1,-1,-1,-1,-1,-1,
	-1, 9,10,11,12,13,14,15,16,-1,17,18,19,20,21,-1,
	22,23,24,25,26,27,28,29,30,31,32,-1,-1,-1,-1,-1,
	-1,33,34,35,36,37,38,39,40,41,42,43,-1,44,45,46,
	47,48,49,50,51,52,53,54,55,56,57,-1,-1,-1,-1,-1,
};

// Encode a byte sequence as a base58-encoded string
std::string EncodeBase58(const unsigned char* begin, const unsigned char* end)
{
    // Skip & count leading zeroes.
    int zeroes = 0;
    int length = 0;
    while (begin != end && *begin == 0) {
        begin += 1;
        zeroes += 1;
    }

    // Allocate enough space in big-endian base58 representation.
    auto base58Size = (end - begin) * 138 / 100 + 1; // log(256) / log(58), rounded up.
    std::vector<unsigned char> b58(base58Size);

    while (begin != end) {
        int carry = *begin;
        int i = 0;
        // Apply "b58 = b58 * 256 + ch".
        for (auto b58it = b58.rbegin(); (carry != 0 || i < length) && (b58it != b58.rend());
             b58it++, i++) {
            carry += 256 * (*b58it);
            *b58it = carry % 58;
            carry /= 58;
        }

        assert(carry == 0);
        length = i;
        begin += 1;
    }

    // Skip leading zeroes in base58 result.
    auto it = b58.begin() + (base58Size - length);
    while (it != b58.end() && *it == 0) {
        it++;
    }

    // Translate the result into a string.
    std::string str;
    str.reserve(zeroes + (b58.end() - it));
    str.assign(zeroes, digits[0]);
    while (it != b58.end()) {
        str += digits[*it];
        it += 1;
    }
    return str;
}

// Encode a byte vector as a base58-encoded string
std::string EncodeBase58(const std::vector<unsigned char>& vch)
{
    return EncodeBase58(&vch[0], &vch[0] + vch.size());
}

// Decode a base58-encoded string psz into byte vector vchRet
// returns true if decoding is successful
bool DecodeBase58(const char* psz, std::vector<unsigned char>& vchRet)
{
    const auto* it = psz;
    const char* end = it + strlen(psz);

    // Skip leading spaces.
    it = std::find_if_not(it, end, [](char c) { return std::isspace(c);});

    // Skip and count leading zeros.
    std::size_t zeroes = 0;
    std::size_t length = 0;
    while (it != end && *it == digits[0]) {
        zeroes += 1;
        it += 1;
    }

    // Allocate enough space in big-endian base256 representation.
    std::size_t base258Size = (end - it) * 733 / 1000 + 1; // log(58) / log(256), rounded up.
    std::vector<unsigned char> b256(base258Size);

    // Process the characters.
   while (it != end && !std::isspace(*it)) {
        if (static_cast<unsigned char>(*it) >= 128) {
            // Invalid b58 character
            return false;
        }

        // Decode base58 character
        int carry = characterMap[static_cast<unsigned char>(*it)];
        if (carry == -1) {
            // Invalid b58 character
            return false;
        }

        std::size_t i = 0;
        for (auto b256it = b256.rbegin(); (carry != 0 || i < length) && (b256it != b256.rend());
             ++b256it, ++i) {
            carry += 58 * (*b256it);
            *b256it = static_cast<uint8_t>(carry % 256);
            carry /= 256;
        }
        assert(carry == 0);
        length = i;
        it += 1;
    }

    // Skip trailing spaces.
    it = std::find_if_not(it, end, [](char c) { return std::isspace(c);});
    if (it != end) {
        // Extra charaters at the end
        return false;
    }

    // Skip leading zeroes in b256.
    auto b256it = b256.begin() + (base258Size - length);
    while (b256it != b256.end() && *b256it == 0) {
        b256it++;
    }

    // Copy result into output vector.
    vchRet.reserve(zeroes + (b256.end() - b256it));
    vchRet.assign(zeroes, 0x00);
    std::copy(b256it, b256.end(), std::back_inserter(vchRet));

    return true;
}

// Decode a base58-encoded string str into byte vector vchRet
// returns true if decoding is successful
bool DecodeBase58(const std::string& str, std::vector<unsigned char>& vchRet)
{
    return DecodeBase58(str.c_str(), vchRet);
}

// Encode a byte vector to a base58-encoded string, including checksum
std::string EncodeBase58Check(const std::vector<unsigned char>& vchIn)
{
    // add 4-byte hash check to the end
    std::vector<unsigned char> vch(vchIn);
    uint256 hash = Hash(vch.begin(), vch.end());
    vch.insert(vch.end(), (unsigned char*)&hash, (unsigned char*)&hash + 4);
    return EncodeBase58(vch);
}

// Decode a base58-encoded string psz that includes a checksum, into byte vector vchRet
// returns true if decoding is successful
bool DecodeBase58Check(const char* psz, std::vector<unsigned char>& vchRet)
{
    if (!DecodeBase58(psz, vchRet))
        return false;
    if (vchRet.size() < 4)
    {
        vchRet.clear();
        return false;
    }
    uint256 hash = Hash(vchRet.begin(), vchRet.end()-4);
    if (memcmp(&hash, &vchRet.end()[-4], 4) != 0)
    {
        vchRet.clear();
        return false;
    }
    vchRet.resize(vchRet.size()-4);
    return true;
}

// Decode a base58-encoded string str that includes a checksum, into byte vector vchRet
// returns true if decoding is successful
bool DecodeBase58Check(const std::string& str, std::vector<unsigned char>& vchRet)
{
    return DecodeBase58Check(str.c_str(), vchRet);
}

    CBase58Data::CBase58Data()
    {
        nVersion = 0;
        vchData.clear();
    }

    CBase58Data::~CBase58Data()
    {
        // zero the memory, as it may contain sensitive data
        if (!vchData.empty())
            OPENSSL_cleanse(&vchData[0], vchData.size());
    }

    void CBase58Data::SetData(int nVersionIn, const void* pdata, size_t nSize)
    {
        nVersion = nVersionIn;
        vchData.resize(nSize);
        if (!vchData.empty())
            memcpy(&vchData[0], pdata, nSize);
    }
    
    const std::vector<unsigned char> &CBase58Data::GetData() const
    {
        return vchData;
    }

    void CBase58Data::SetData(int nVersionIn, const unsigned char *pbegin, const unsigned char *pend)
    {
        SetData(nVersionIn, (void*)pbegin, pend - pbegin);
    }

    bool CBase58Data::SetString(const char* psz)
    {
        std::vector<unsigned char> vchTemp;
        DecodeBase58Check(psz, vchTemp);
        if (vchTemp.empty())
        {
            vchData.clear();
            nVersion = 0;
            return false;
        }
        nVersion = vchTemp[0];
        vchData.resize(vchTemp.size() - 1);
        if (!vchData.empty())
            memcpy(&vchData[0], &vchTemp[1], vchData.size());
        OPENSSL_cleanse(&vchTemp[0], vchData.size());
        return true;
    }

    bool CBase58Data::SetString(const std::string& str)
    {
        return SetString(str.c_str());
    }

    std::string CBase58Data::ToString() const
    {
        std::vector<unsigned char> vch{nVersion};
        vch.insert(vch.end(), vchData.begin(), vchData.end());
        return EncodeBase58Check(vch);
    }

    int CBase58Data::CompareTo(const CBase58Data& b58) const
    {
        if (nVersion < b58.nVersion) return -1;
        if (nVersion > b58.nVersion) return  1;
        if (vchData < b58.vchData)   return -1;
        if (vchData > b58.vchData)   return  1;
        return 0;
    }

    namespace {
        class CBitcoinAddressVisitor {
        private:
            CBitcoinAddress *addr;
        public:
            explicit CBitcoinAddressVisitor(CBitcoinAddress *addrIn) : addr(addrIn) { }

            bool operator()(const CKeyID &id) const { return addr->Set(id); }
            bool operator()(const CScriptID &id) const { return addr->Set(id); }
            bool operator()(const CMalleablePubKey &mpk) const { return addr->Set(mpk); }
            bool operator()([[maybe_unused]] const CNoDestination &no) const { return false; }
        };
    } // namespace

    bool CBitcoinAddress::Set(const CKeyID &id) {
        SetData(fTestNet ? PUBKEY_ADDRESS_TEST : PUBKEY_ADDRESS, &id, 20);
        return true;
    }

    bool CBitcoinAddress::Set(const CScriptID &id) {
        SetData(fTestNet ? SCRIPT_ADDRESS_TEST : SCRIPT_ADDRESS, &id, 20);
        return true;
    }

    bool CBitcoinAddress::Set(const CTxDestination &dest)
    {
        return std::visit(CBitcoinAddressVisitor(this), dest);
    }

    bool CBitcoinAddress::Set(const CMalleablePubKey &mpk) {
        std::vector<unsigned char> vchPubkeyPair = mpk.Raw();
        SetData(fTestNet ? PUBKEY_PAIR_ADDRESS_TEST : PUBKEY_PAIR_ADDRESS, &vchPubkeyPair[0], 68);
        return true;
    }

    bool CBitcoinAddress::Set(const CBitcoinAddress &dest)
    {
        nVersion = dest.nVersion;
        vchData = dest.vchData;
        return true;
    }

    bool CBitcoinAddress::IsValid() const
    {
        unsigned int nExpectedSize = 20;
        bool fExpectTestNet = false;
        bool fSimple = true;
        switch(nVersion)
        {
            case PUBKEY_PAIR_ADDRESS:
                nExpectedSize = 68; // Serialized pair of public keys
                fExpectTestNet = false;
                fSimple = false;
                break;
            case PUBKEY_ADDRESS:
                nExpectedSize = 20; // Hash of public key
                fExpectTestNet = false;
                break;
            case SCRIPT_ADDRESS:
                nExpectedSize = 20; // Hash of CScript
                fExpectTestNet = false;
                break;

            case PUBKEY_PAIR_ADDRESS_TEST:
                nExpectedSize = 68;
                fExpectTestNet = true;
                fSimple = false;
                break;
            case PUBKEY_ADDRESS_TEST:
                nExpectedSize = 20;
                fExpectTestNet = true;
                break;
            case SCRIPT_ADDRESS_TEST:
                nExpectedSize = 20;
                fExpectTestNet = true;
                break;

            default:
                return false;
        }

        // Basic format sanity check
        bool fSeemsSane = (fExpectTestNet == fTestNet && vchData.size() == nExpectedSize);

        if (fSeemsSane && !fSimple)
        {
            // Perform additional checking
            //    for pubkey pair addresses
            CMalleablePubKey mpk;
            mpk.setvch(vchData);
            return mpk.IsValid();
        }
        else
            return fSeemsSane;
    }

    CTxDestination CBitcoinAddress::Get() const {
        if (!IsValid())
            return CNoDestination();
        switch (nVersion) {
        case PUBKEY_ADDRESS:
        case PUBKEY_ADDRESS_TEST: {
            uint160 id;
            memcpy(&id, &vchData[0], 20);
            return CKeyID(id);
        }
        case SCRIPT_ADDRESS:
        case SCRIPT_ADDRESS_TEST: {
            uint160 id;
            memcpy(&id, &vchData[0], 20);
            return CScriptID(id);
        }
        }
        return CNoDestination();
    }

    bool CBitcoinAddress::GetKeyID(CKeyID &keyID) const {
        if (!IsValid())
            return false;
        switch (nVersion) {
        case PUBKEY_ADDRESS:
        case PUBKEY_ADDRESS_TEST: {
            uint160 id;
            memcpy(&id, &vchData[0], 20);
            keyID = CKeyID(id);
            return true;
        }
        case PUBKEY_PAIR_ADDRESS:
        case PUBKEY_PAIR_ADDRESS_TEST:
        {
            CMalleablePubKey mPubKey;
            mPubKey.setvch(vchData);
            keyID = mPubKey.GetID();
            return true;
        }
        default: return false;
        }
    }

    bool CBitcoinAddress::IsScript() const {
        if (!IsValid())
            return false;
        switch (nVersion) {
        case SCRIPT_ADDRESS:
        case SCRIPT_ADDRESS_TEST: {
            return true;
        }
        default: return false;
        }
    }

    bool CBitcoinAddress::IsPubKey() const {
        if (!IsValid())
            return false;
        switch (nVersion) {
        case PUBKEY_ADDRESS:
        case PUBKEY_ADDRESS_TEST: {
            return true;
        }
        default: return false;
        }
    }
    
    bool CBitcoinAddress::IsPair() const {
        if (!IsValid())
            return false;
        switch (nVersion) {
        case PUBKEY_PAIR_ADDRESS:
        case PUBKEY_PAIR_ADDRESS_TEST: {
            return true;
        }
        default: return false;
        }
    }

    void CBitcoinSecret::SetSecret(const CSecret& vchSecret, bool fCompressed)
    {
        assert(vchSecret.size() == 32);
        SetData(128 + (fTestNet ? CBitcoinAddress::PUBKEY_ADDRESS_TEST : CBitcoinAddress::PUBKEY_ADDRESS), &vchSecret[0], vchSecret.size());
        if (fCompressed)
            vchData.push_back(1);
    }

    CSecret CBitcoinSecret::GetSecret(bool &fCompressedOut)
    {
        CSecret vchSecret;
        vchSecret.resize(32);
        memcpy(&vchSecret[0], &vchData[0], 32);
        fCompressedOut = vchData.size() == 33;
        return vchSecret;
    }

    bool CBitcoinSecret::IsValid() const
    {
        bool fExpectTestNet = false;
        switch(nVersion)
        {
            case (128 + CBitcoinAddress::PUBKEY_ADDRESS):
                break;

            case (128 + CBitcoinAddress::PUBKEY_ADDRESS_TEST):
                fExpectTestNet = true;
                break;

            default:
                return false;
        }
        return fExpectTestNet == fTestNet && (vchData.size() == 32 || (vchData.size() == 33 && vchData[32] == 1));
    }

    bool CBitcoinSecret::SetString(const char* pszSecret)
    {
        return CBase58Data::SetString(pszSecret) && IsValid();
    }

    bool CBitcoinSecret::SetString(const std::string& strSecret)
    {
        return SetString(strSecret.c_str());
    }

    CBitcoinSecret::CBitcoinSecret(const CSecret& vchSecret, bool fCompressed)
    {
        SetSecret(vchSecret, fCompressed);
    }
