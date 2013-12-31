// Copyright (c) 2013 The NovaCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_KEYCHAIN_H
#define BITCOIN_KEYCHAIN_H

#include <iostream>
#include <cassert>

#include <vector>
#include <sstream>

#include "key.h"
#include "base58.h"
#include "util.h"
#include "allocators.h"


const uint160 ROOT_PARENT(ParseHex("817c1eea957014843e1f3cf3b763955c58aedfc3"));
const uint160 RAND_PARENT(ParseHex("14c444143d296cf94203c9ab8fe249a7ae4cb8a8"));

inline unsigned int P(uint32_t i)   { return (0x80000000 | i); }
inline bool         isP(uint32_t i) { return (0x80000000 & i); }

class keychain_error : public std::runtime_error
{
public:
    explicit keychain_error(const std::string& str) : std::runtime_error(str) {}
};

class CNodeMeta
{
public:
    CNodeMeta()
    {
        nDepth = 0;
        nSequence = 0;
        nDerivationMethod = 0;
        parentKeyID = CKeyID(RAND_PARENT);
        std::fill(vchChainCode.begin(), vchChainCode.end(), 0);
    }

    unsigned int nDepth;
    unsigned int nSequence;
    unsigned short nDerivationMethod;

    CKeyID parentKeyID;
    std::vector<unsigned char> vchChainCode;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(nDerivationMethod);   // 2 bytes
        READWRITE(nDepth);              // 4 bytes
        READWRITE(nSequence);           // 4 bytes
        READWRITE(vchChainCode);        // 32 bytes
        READWRITE(parentKeyID);         // 20 bytes
    )
};

class CDerivation
{
    friend class CKDSeed;
    friend class CBaseChain;
    friend class CPubChain;
    friend class CPrivChain;

public:
    void setDerivationMethod(unsigned short pnDerivationMethod);
    unsigned short getDerivationMethod() const  { return nDerivationMethod; }

template<class T1, class T2>
    void getDigest(const T1& vchData, T2& vchDigest) const;

template<class T1, class T2, class T3>
    void getDigest(const T1& vchKey, const T2& vchData, T3& vchDigest) const;

private:
    unsigned short nDerivationMethod;
};

// Seed object, allows us to create master key 
//    and master code from a supplied seed.
class CKDSeed : public CDerivation
{
public:

    static unsigned short getVersion() { return !fTestNet ? CURRENT_VERSION : CURRENT_VERSION_TEST; }

    CKDSeed(unsigned short pnDerivationMethod = 0);

    void setSeed();
    void setSeed(const SecureString& seed);

    CBitcoinSecret getMasterSecret() const;
    CKey getMasterKey() const;
    CPubKey getMasterPubKey() const;
    CKeyID getMasterID()  const;
    const std::vector<unsigned char>& getMasterChainCode() const { return vchChainMasterCode; }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(nDerivationMethod);
        READWRITE(vchChainMasterSecret);
        READWRITE(vchChainMasterPublic);
        READWRITE(vchChainMasterCode);
    )

private:
    CSecret vchChainMasterSecret;
    std::vector<unsigned char> vchChainMasterCode;
    std::vector<unsigned char> vchChainMasterPublic;

    static const unsigned short CURRENT_VERSION = 0x97CF;
    static const unsigned short CURRENT_VERSION_TEST = 0xC21E;
};

// Base class for a public and private chain objects.
class CBaseChain : public CDerivation
{
    friend class CPubChain;
    friend class CPrivChain;

public:
    bool isValid() const { return fValid; };
    const std::vector<unsigned char>& getChainCode() const { return vchChainCode; }

    unsigned int  getSequence()          const { return nSequence; }
    unsigned int  getDepth()             const { return nDepth;    }

    CKeyID        getKeyID()             const { return CKeyID(uint160(vchPublicKeyHash));       }
    CKeyID        getParentID()          const { return CKeyID(uint160(vchParentHash)); }
    CPubKey       getPubKey()            const { return CPubKey(vchPublicKey); }
    CNodeMeta     getNodeMeta()          const;

private:

    // Chain params (node depth, our number, chain code and parent node public key hash)
    unsigned int nDepth;
    unsigned int nSequence;
    std::vector<unsigned char> vchChainCode;
    std::vector<unsigned char> vchParentHash;

    std::vector<unsigned char> vchPublicKey;
    std::vector<unsigned char> vchPublicKeyHash;

    // Validation result
    bool fValid;

    // Check vector consistency
template<class T>
    bool checkVector(T vchVector, bool fCheckZero = false) const;
};

// Public chain class
class CPubChain : public CBaseChain//, public CDerivation
{
public:
    static unsigned short getVersion() { return !fTestNet ? CURRENT_VERSION : CURRENT_VERSION_TEST; }

    // Constructors
    CPubChain() { }
    CPubChain(const CPubChain& pCSource);
    CPubChain(const std::vector<unsigned char>& pvchKey, const std::vector<unsigned char>& pvchChainCode, const std::vector<unsigned char>& pvchParentHash, unsigned int pnSequence = 0, unsigned int pnDepth = 0, unsigned short pnDerivationMethod = 0);

    // Operators
    CPubChain& operator=(const CPubChain& pcRightOperand);
    bool operator==(const CPubChain& pcRightOperand) const;
    bool operator!=(const CPubChain& pcRightOperand) const;

    // Accessor Methods
    CPubChain getChild(unsigned int pnSequence) const;

    // Public chain object serialization. We should be able to 
    //   implement import and export functionality, so that's neccessary.
    IMPLEMENT_SERIALIZE
    (
        READWRITE(nDerivationMethod);  // 2 bytes
        READWRITE(nDepth);             // 4 bytes
        READWRITE(nSequence);          // 4 bytes
        READWRITE(vchChainCode);       // 32 bytes
        READWRITE(vchParentHash);      // 20 bytes
        READWRITE(vchPublicKey);       // 33 bytes
    )

private:
    // Regenerate pubkey hash
    void updatePublicKeyHash();

    static const unsigned short CURRENT_VERSION = 0xA7CE;
    static const unsigned short CURRENT_VERSION_TEST = 0xD21F;
};

class CPrivChain : public CBaseChain
{
public:
    static unsigned short getVersion() { return !fTestNet ? 0xBADE : 0xE22F; }

    // Constructors
    CPrivChain() { }
    CPrivChain(const std::string& sSource);
    CPrivChain(const CPrivChain& source);
    CPrivChain(const CSecret& pvchKey, const std::vector<unsigned char>& pvchChainCode, const std::vector<unsigned char>& pvchParentHash, unsigned int pnSequence = 0, unsigned int pnDepth = 0, unsigned short pnDerivationMethod = 0);

    // Operators
    CPrivChain& operator=(const CPrivChain& pcRightOperand);
    bool operator==(const CPrivChain& pcRightOperand) const;
    bool operator!=(const CPrivChain& pcRightOperand) const;

    // Accessor Methods
    CBitcoinSecret getSecret() const { return CBitcoinSecret(vchPrivateKey, true); }
    CKey getKey() const;
    CPrivChain     getChild(unsigned int pnSequence) const;
    CPubChain      getPublic() const;

    // Private chain object serialization. We should be able to 
    //   implement import and export functionality, so that's neccessary.
    IMPLEMENT_SERIALIZE
    (
        READWRITE(nDerivationMethod);  // 2 bytes
        READWRITE(nDepth);             // 4 byte
        READWRITE(nSequence);          // 4 bytes
        READWRITE(vchChainCode);       // 32 bytes
        READWRITE(vchPrivateKey);      // 32 bytes
        READWRITE(vchParentHash);      // 20 bytes
    )

    // Serialize method
    std::string ToString() const;

private:
    // Secret key
    CSecret vchPrivateKey;

    // Regenerate public key and public key hash
    void updatePublicKey();

    // Check secret consistency and derive new key if successful
    bool derivateSecret(const CSecret pvchSecret, const std::vector<unsigned char> pvchLeft32, CSecret& pvchChildSecret) const;

};

#endif
