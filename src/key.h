// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_KEY_H
#define BITCOIN_KEY_H

#include <stdexcept>
#include <vector>

#include "allocators.h"
#include "serialize.h"
#include "uint256.h"
#include "util.h"

#include <openssl/ec.h> // for EC_KEY definition

// secp160k1
// const unsigned int PRIVATE_KEY_SIZE = 192;
// const unsigned int PUBLIC_KEY_SIZE  = 41;
// const unsigned int SIGNATURE_SIZE   = 48;
//
// secp192k1
// const unsigned int PRIVATE_KEY_SIZE = 222;
// const unsigned int PUBLIC_KEY_SIZE  = 49;
// const unsigned int SIGNATURE_SIZE   = 57;
//
// secp224k1
// const unsigned int PRIVATE_KEY_SIZE = 250;
// const unsigned int PUBLIC_KEY_SIZE  = 57;
// const unsigned int SIGNATURE_SIZE   = 66;
//
// secp256k1:
// const unsigned int PRIVATE_KEY_SIZE = 279;
// const unsigned int PUBLIC_KEY_SIZE  = 65;
// const unsigned int SIGNATURE_SIZE   = 72;
//
// see www.keylength.com
// script supports up to 75 for single byte push

class key_error : public std::runtime_error
{
public:
    explicit key_error(const std::string& str) : std::runtime_error(str) {}
};

/** A reference to a CKey: the Hash160 of its serialized public key */
class CKeyID : public uint160
{
public:
    CKeyID() : uint160(0) { }
    CKeyID(const uint160 &in) : uint160(in) { }
};

/** A reference to a CScript: the Hash160 of its serialization (see script.h) */
class CScriptID : public uint160
{
public:
    CScriptID() : uint160(0) { }
    CScriptID(const uint160 &in) : uint160(in) { }
};

/** An encapsulated public key. */
class CPubKey {
private:
    std::vector<unsigned char> vchPubKey;
    friend class CKey;

public:
    CPubKey() { }
    CPubKey(const std::vector<unsigned char> &vchPubKeyIn) : vchPubKey(vchPubKeyIn) { }
    friend bool operator==(const CPubKey &a, const CPubKey &b) { return a.vchPubKey == b.vchPubKey; }
    friend bool operator!=(const CPubKey &a, const CPubKey &b) { return a.vchPubKey != b.vchPubKey; }
    friend bool operator<(const CPubKey &a, const CPubKey &b) { return a.vchPubKey < b.vchPubKey; }

    IMPLEMENT_SERIALIZE(
        READWRITE(vchPubKey);
    )

    CKeyID GetID() const {
        return CKeyID(Hash160(vchPubKey));
    }

    uint256 GetHash() const {
        return Hash(vchPubKey.begin(), vchPubKey.end());
    }

    bool IsValid() const {
        return vchPubKey.size() == 33 || vchPubKey.size() == 65;
    }

    bool IsCompressed() const {
        return vchPubKey.size() == 33;
    }

    std::vector<unsigned char> Raw() const {
        return vchPubKey;
    }
};


// secure_allocator is defined in allocators.h
// CPrivKey is a serialized private key, with all parameters included (279 bytes)
typedef std::vector<unsigned char, secure_allocator<unsigned char> > CPrivKey;
// CSecret is a serialization of just the secret parameter (32 bytes)
typedef std::vector<unsigned char, secure_allocator<unsigned char> > CSecret;

/** An encapsulated OpenSSL Elliptic Curve key (public and/or private) */
class CKey
{
protected:
    EC_KEY* pkey;
    bool fSet;
    bool fCompressedPubKey;

    void SetCompressedPubKey();

public:

    void Reset();

    CKey();
    CKey(const CKey& b);

    CKey& operator=(const CKey& b);

    ~CKey();

    bool IsNull() const;
    bool IsCompressed() const;

    void MakeNewKey(bool fCompressed);
    bool SetPrivKey(const CPrivKey& vchPrivKey);
    bool SetSecret(const CSecret& vchSecret, bool fCompressed = false);
    CSecret GetSecret() const;
    CSecret GetSecret(bool &fCompressed) const;
    CPrivKey GetPrivKey() const;
    bool SetPubKey(const CPubKey& vchPubKey);
    CPubKey GetPubKey() const;

    bool Sign(uint256 hash, std::vector<unsigned char>& vchSig);

    // create a compact signature (65 bytes), which allows reconstructing the used public key
    // The format is one header byte, followed by two times 32 bytes for the serialized r and s values.
    // The header byte: 0x1B = first key with even y, 0x1C = first key with odd y,
    //                  0x1D = second key with even y, 0x1E = second key with odd y
    bool SignCompact(uint256 hash, std::vector<unsigned char>& vchSig);

    // reconstruct public key from a compact signature
    // This is only slightly more CPU intensive than just verifying it.
    // If this function succeeds, the recovered public key is guaranteed to be valid
    // (the signature is a valid signature of the given data for that key)
    bool SetCompactSignature(uint256 hash, const std::vector<unsigned char>& vchSig);

    bool Verify(uint256 hash, const std::vector<unsigned char>& vchSig);

    // Verify a compact signature
    bool VerifyCompact(uint256 hash, const std::vector<unsigned char>& vchSig);

    bool IsValid();
};

// ECDSA point class, used for public keys derivation.
class CPoint
{
public:
    CPoint()
    {
        init();
    }

    // Construct a new CPoint object as a copy of supplied one.
    CPoint(const CPoint& source)
    {
        init();
        if (!EC_GROUP_copy(group, source.group)) throw std::runtime_error("CPoint::CPoint(const CPoint&) - EC_GROUP_copy failed.");
        if (!EC_POINT_copy(point, source.point)) throw std::runtime_error("CPoint::CPoint(const CPoint&) - EC_POINT_copy failed.");
    }

    // Construct a new CPoint object and init it with the supplied public key bytes.
    CPoint(const std::vector<unsigned char>& vchPublicKey)
    {
        init();
        this->setPublicKey(vchPublicKey);
    }

    ~CPoint()
    {
        if (point) EC_POINT_free(point);
        if (group) EC_GROUP_free(group);
        if (ctx)   BN_CTX_free(ctx);
    }

    CPoint& operator=(const CPoint& rhs)
    {
        if (!EC_GROUP_copy(group, rhs.group)) throw std::runtime_error("CPoint::operator= - EC_GROUP_copy failed.");
        if (!EC_POINT_copy(point, rhs.point)) throw std::runtime_error("CPoint::operator= - EC_POINT_copy failed.");
    }


    CPoint& operator+=(const CPoint& rhs)
    {
        if (!EC_POINT_add(group, point, point, rhs.point, ctx)) {
            throw std::runtime_error("CPoint::operator+= - EC_POINT_add failed.");
        }
    }

    CPoint& operator*=(const std::vector<unsigned char>& rhs)
    {
        BIGNUM* bn = BN_bin2bn(&rhs[0], rhs.size(), NULL);
        if (!bn) {
            throw std::runtime_error("CPoint::operator*=  - BN_bin2bn failed.");
        }

        int rval = EC_POINT_mul(group, point, NULL, point, bn, ctx);
        BN_clear_free(bn);

        if (rval == 0) {
            throw std::runtime_error("CPoint::operator*=  - EC_POINT_mul failed.");
        }
    }

    const CPoint operator+(const CPoint& rhs) const { return CPoint(*this) += rhs; }
    const CPoint operator*(const std::vector<unsigned char>& rhs) const { return CPoint(*this) *= rhs; }

    // Computes n*G + K where K is this and G is the group generator
    void generator_mul(const std::vector<unsigned char>& n);
    bool is_at_infinity() const { return EC_POINT_is_at_infinity(group, point); }

    void setPublicKey(const std::vector<unsigned char>& vch);
    std::vector<unsigned char> getPublicKey() const;

protected:
    void init();

private:
    EC_GROUP* group;
    EC_POINT* point;
    BN_CTX*   ctx;
};

#endif
