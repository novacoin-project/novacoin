// Copyright (c) 2013 The NovaCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sstream>
#include <stdexcept>

#include <openssl/bn.h>
#include <openssl/rand.h>

#include "key.h"
#include "util.h"
#include "keychain.h"

const std::vector<unsigned char> CKD_SEED(ParseHex("4A75737420736F6D6520707265666978"));
const std::vector<unsigned char> CURVE_ORDER(ParseHex("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141"));

void CDerivation::setDerivationMethod(unsigned short pnDerivationMethod) {
    if (pnDerivationMethod != 0)
        throw new keychain_error("CDerivation::setDerivationMethod() : Only HMAC-SHA512 is implemented yet.");
    nDerivationMethod = pnDerivationMethod;
}

template<class T1, class T2>
void CDerivation::getDigest(const T1& vchData, T2& vchDigest) const
{
    getDigest(CKD_SEED, vchData, vchDigest);
}

template<class T1, class T2, class T3>
void CDerivation::getDigest(const T1& vchKey, const T2& vchData, T3& vchDigest) const
{
    unsigned char* digest = HMAC(EVP_sha512(), (unsigned char*)&vchKey[0], vchKey.size(), (unsigned char*)&vchData[0], vchData.size(), NULL, NULL);
    vchDigest = T3(digest, digest+64);
}

// Construct a new seed object
CKDSeed::CKDSeed(unsigned short pnDerivationMethod)
{
    setDerivationMethod(pnDerivationMethod);
}

// Set random root
void CKDSeed::setSeed()
{
    RandAddSeedPerfmon();

    CKey key;
    key.MakeNewKey(true);

    CSecret privKey = key.GetSecret();
    std::vector<unsigned char> hmac(64);

    getDigest(privKey, hmac);
    vchChainMasterSecret = privKey;

    vchChainMasterCode.assign(hmac.begin() + 32, hmac.end());
    vchChainMasterPublic = key.GetPubKey().Raw();
}

// Set supplied seed
void CKDSeed::setSeed(const SecureString& seed)
{
    if (seed.length() == 0)
    {
        setSeed();
        return;
    }

    CSecret hmac(64);
    getDigest(seed, hmac);

    vchChainMasterSecret.assign(hmac.begin(), hmac.begin() + 32);
    vchChainMasterCode.assign(hmac.begin() + 32, hmac.end());

    CKey key;

    if (key.SetSecret(vchChainMasterSecret, true))
    {
        vchChainMasterPublic = key.GetPubKey().Raw();
    }
    else
        throw keychain_error("CKDSeed::setSeed() : SetSecret failure, try again with another seed.");

}

CBitcoinSecret CKDSeed::getMasterSecret() const
{
    if (vchChainMasterSecret.size() != 32)
        throw keychain_error("CKDSeed::getMasterSecret() : you have to initialize object first.");

    return CBitcoinSecret(vchChainMasterSecret, true);
}

CKey CKDSeed::getMasterKey() const
{
    if (vchChainMasterSecret.size() != 32)
        throw keychain_error("CKDSeed::getMasterKey() : you have to initialize object first.");

    CKey curvekey;
    curvekey.SetSecret(vchChainMasterSecret, true);

    return curvekey;
}

CKeyID CKDSeed::getMasterID() const
{
    if (vchChainMasterPublic.size() != 33)
        throw keychain_error("CKDSeed::getMasterID() : you have to initialize object first.");

    return CPubKey(vchChainMasterPublic).GetID();
}

CPubKey CKDSeed::getMasterPubKey() const
{
    if (vchChainMasterPublic.size() != 33)
        throw keychain_error("CKDSeed::getMasterID() : you have to initialize object first.");

    return CPubKey(vchChainMasterPublic);
}

// Check vector consistency
template<class T>
bool CBaseChain::checkVector(T vchVector, bool fCheckZero) const
{
    bool fSuccess = true; // Valid key

    BIGNUM *bnVector = BN_bin2bn(&vchVector[0], vchVector.size(), NULL);
    BIGNUM *bnCurveOrder = BN_bin2bn(&CURVE_ORDER[0], CURVE_ORDER.size(), NULL);

    if (!bnVector) { throw keychain_error("CheckVector() : bnKey allocation error."); }
    if (!bnCurveOrder) { throw keychain_error("CheckVector() : bnCurveOrder allocation error."); }

    if (BN_cmp(bnVector, bnCurveOrder) >= 0 || (fCheckZero && BN_is_zero(bnVector))) {
        fSuccess = false; // Invalid key
    }

    BN_free(bnVector);
    BN_free(bnCurveOrder);

    return fSuccess;
}

// Construct an instance of public chain object as a copy of supplied one
CPubChain::CPubChain(const CPubChain& pCSource)
{
    fValid = pCSource.fValid;
    if (!fValid) return;

    nDepth = pCSource.nDepth;
    nSequence = pCSource.nSequence;
    vchChainCode = pCSource.vchChainCode;
    vchPublicKey = pCSource.vchPublicKey;
    vchParentHash = pCSource.vchParentHash;

    updatePublicKeyHash();
}

// Construct a new instance of public chain object using supplied data:
//
// * pvchKey - ECDSA "compressed" public key;
// * pvchChainCode - chain code, additional 256 bits of entropy added to key, root 
//   node of chain should contain zero value here;
// * pvchParentHash - parent public key hash, result of Hash160(parentPubKey)
// * pnSequence - child number, unsigned value from 0 to UINT_MAX;
// * pnDepth - chain depth
CPubChain::CPubChain(const std::vector<unsigned char>& pvchKey, const std::vector<unsigned char>& pvchChainCode, const std::vector<unsigned char>& pvchParentHash, unsigned int pnSequence, unsigned int pnDepth, unsigned short pnDerivationMethod)
{
    if (pvchChainCode.size() != 32) {
        throw keychain_error("CPubChain::CPubChain() : Invalid chain code.");
    }

    if (pvchKey.size() != 33) {
        throw keychain_error("CPubChain::CPubChain() : Invalid public key.");
    }

    vchPublicKey = pvchKey;
    vchParentHash = pvchParentHash;
    vchChainCode = pvchChainCode;
    nDerivationMethod = pnDerivationMethod;
    nSequence = pnSequence;
    nDepth = pnDepth;

    updatePublicKeyHash();

    fValid = true;
}

CPubChain& CPubChain::operator=(const CPubChain& pcRightOperand)
{

    if (pcRightOperand.fValid) {
        fValid = pcRightOperand.fValid;
        nDepth = pcRightOperand.nDepth;
        nSequence = pcRightOperand.nSequence;
        vchChainCode = pcRightOperand.vchChainCode;
        vchPublicKey = pcRightOperand.vchPublicKey;
        nDerivationMethod = pcRightOperand.nDerivationMethod;
        vchParentHash = pcRightOperand.vchParentHash;

        updatePublicKeyHash();
    }

    return *this;
}

bool CPubChain::operator==(const CPubChain& pcRightOperand) const
{
    return (fValid && pcRightOperand.fValid &&
            nDepth == pcRightOperand.nDepth &&
            nSequence == pcRightOperand.nSequence &&
            vchChainCode == pcRightOperand.vchChainCode &&
            vchPublicKey == pcRightOperand.vchPublicKey &&
            nDerivationMethod == pcRightOperand.nDerivationMethod &&
            vchParentHash == pcRightOperand.vchParentHash &&
            vchPublicKeyHash == pcRightOperand.vchPublicKeyHash);
}

bool CPubChain::operator!=(const CPubChain& pcRightOperand) const
{
    return !(*this == pcRightOperand);
}

// Public key derivation using supplied i, provides a basic CKD'(m, i) implementation.
//
// NOTE: CKD'(m, i) doesn't support private derivation.
CPubChain CPubChain::getChild(unsigned int pnSequence) const
{
    if (!fValid) {
        throw keychain_error("CPubChain::getChild() : CPubChain is invalid.");
    }

    if (isP(pnSequence)) {
        throw keychain_error(
            "CPubChain::getChild() : CPubChain doesn't support "
            "private derivation, use CPrivChain instance do perform"
            " private derivation.");
    }

    CPubChain child;
    child.fValid = false;

    // Using public key to provide deterministic entropy source for a child 
    //    and grandchild keys generation.
    //
    // Keychain generation process is quite simple and could be split into a few simple steps:
    //
    // 1. Get a digest of our master key using a 32 byte zero filled vector as the key:
    //
    //    LR1 = HMAC(R(0), K(0) + EXT(0))
    //
    // 2. Split LR1 vector into two parts:
    //
    //    R1 it's a right 32 bytes of LR(1);
    //    L1 it's a left 32 bytes of LR(1);
    //
    // 3. Check L1 vector to satisfy this condition:
    //
    //    L(1) != 0 && L(1) < CURVE_ORDER
    //
    // If L(1) is zero or isn't less than CURVE_ORDER then throw an error or try
    //    another parent key or i value.
    //
    // 4. Create the new ECDSA group instance G, and then add new point P.
    //
    // 5. Trying to compute new key K(1):
    //
    //    K(1) = L(1) * G + K(0)
    // 
    // 6. Check that K(1) is a finite value. If K(1) is INF then throw an error or 
    //    try another parent key or i value.
    //
    // 7. Store K(1), R(1), i and continue:
    //
    // A1. Get a digest of K(1) using R(1) vector as the key:
    //
    //    LR(2) = HMAC(R(1), K(1) + EXT(1))
    //
    // A2. Split LR(2) vector into two parts:
    //
    //    R(2) it's a right 32 bytes of LR(2);
    //    L(2) it's a left 32 bytes of LR(2);
    //
    // A3. Check L2 vector to satisfy this condition:
    //
    //    L(2) != 0 && L(2) < CURVE_ORDER
    //
    // If L(2) is zero or isn't less than CURVE_ORDER then throw an 
    //    error or try another parent key or i value.
    //
    // A4. Create the new ECDSA group instance G, and then add new point P.
    //
    // A5. Trying to compute new key K(2):
    //
    //    K(2) = L(2) * G + K(1)
    // 
    // A6. Check that K(2) is a finite value. If K(2) is INF then 
    //    throw an error or try another parent key or i value.
    //
    // A7. Store K(2), R(2), i and continue.
    //
    // [...]
    //
    // N1. Get a digest of K(n-1) using R(n-1) vector as the key:
    //
    //    LR(n) = HMAC(R(n-1), K(n-1) + EXT(n-1))
    //
    // N2. Split LR(n) vector into two parts:
    //
    //    R(n) it's a right 32 bytes of LR(n);
    //    L(n) it's a left 32 bytes of LR(n);
    //
    // N3. Check L(n) vector to satisfy this condition:
    //
    //    L(n) != 0 && L(n) < CURVE_ORDER
    //
    // If L(n) is zero or isn't less than CURVE_ORDER then throw an 
    //    error or try another parent key or i value.
    //
    // N4. Create the new ECDSA group instance G, and then add new point P.
    //
    // N5. Trying to compute new key K(n):
    //
    //    K(n) = L(n) * G + K(n-1)
    // 
    // N6. Check that K(n) is a finite value. If K(n) is INF then 
    //    throw an error or try another parent key or i value.
    //
    // N7. Store K(n), R(n), i and continue.

    std::vector<unsigned char> extended = vchPublicKey;
    extended.push_back(pnSequence >> 24);
    extended.push_back((pnSequence >> 16) & 0xff);
    extended.push_back((pnSequence >>  8) & 0xff);
    extended.push_back(pnSequence & 0xff);

    // HMAC SHA-512 digest of public key, using right half of parent key digest as password.
    // Right half should be filled with zeros for root node generation.
    std::vector<unsigned char> hmac(64);
    getDigest(vchChainCode, extended, hmac);

    // Entropy source for a child key derivation
    std::vector<unsigned char> left32(hmac.begin(), hmac.begin() + 32);

    // Entropy source for a grandchild key generation :)
    std::vector<unsigned char> right32(hmac.begin() + 32, hmac.end());

    if (!checkVector(left32)) return child;

    CPoint K;
    K.setPublicKey(vchPublicKey);
    K.generator_mul(left32);

    if (K.is_at_infinity()) return child;

    child.vchPublicKey = K.getPublicKey();
    child.updatePublicKeyHash();

    child.nDepth = nDepth + 1;
    child.vchParentHash =vchPublicKeyHash;
    child.nSequence = pnSequence;
    child.nDerivationMethod = nDerivationMethod;
    child.vchChainCode = right32;
    child.fValid = true;

    return child;
}

// Regenerate public key hash from vchPublicKey vector value
void CPubChain::updatePublicKeyHash() {
    vchPublicKeyHash = Hash160(vchPublicKey).getvch();
}

// Construct a new instance of private chain object as a copy of a supplied one.
CPrivChain::CPrivChain(const CPrivChain& pCSource)
{
    fValid = pCSource.fValid;
    if (!fValid) return;

    nDepth = pCSource.nDepth;
    vchParentHash = pCSource.vchParentHash;
    nSequence = pCSource.nSequence;
    vchChainCode = pCSource.vchChainCode;
    vchPrivateKey = pCSource.vchPrivateKey;

    updatePublicKey();
}

// Construct a new instance of private chain object from serialized data.
CPrivChain::CPrivChain(const std::string& sSource)
{
    unsigned short nVersion;

    // Decode base58 string
    std::vector<unsigned char> vchTemp;

    DecodeBase58Check(sSource, vchTemp);
    if (vchTemp.empty() || vchTemp.size() != 96)
        return;

    nVersion = vchTemp[0] & 0xff;
    nVersion <<= 8;
    nVersion |= vchTemp[1] & 0xff;

    nDerivationMethod = vchTemp[2] & 0xff;
    nDerivationMethod <<= 8;
    nDerivationMethod |= vchTemp[3] & 0xff;

    nDepth = vchTemp[4] & 0xff;
    nDepth <<= 8;
    nDepth |= vchTemp[5] & 0xff;
    nDepth <<= 8;
    nDepth |= vchTemp[6] & 0xff;
    nDepth <<= 8;
    nDepth |= vchTemp[7] & 0xff;

    nSequence = vchTemp[8]  & 0xff;
    nSequence <<= 8;
    nSequence |= vchTemp[9]  & 0xff;
    nSequence <<= 8;
    nSequence |= vchTemp[10] & 0xff;
    nSequence <<= 8;
    nSequence |= vchTemp[11] & 0xff;

    vchChainCode.assign(vchTemp.begin()  + 12, vchTemp.begin() + 44);
    vchPrivateKey.assign(vchTemp.begin() + 44, vchTemp.begin() + 76);
    vchParentHash.assign(vchTemp.begin() + 76, vchTemp.begin() + 96);

    updatePublicKey();

    fValid = true;
}

// Construct a new instance of private chain object using supplied data:
//
// * pvchKey - ECDSA private key;
// * pvchChainCode - chain code, additional 256 bits of entropy added to key, root 
//   node of chain should contain zero value here;
// * pvchParentHash - parent public key hash, result of Hash160(parentPubKey)
// * pnSequence - child number, unsigned value from 0 to UINT_MAX;
// * pnDepth - chain depth
CPrivChain::CPrivChain(const CSecret& pvchKey, const std::vector<unsigned char>& pvchChainCode, const std::vector<unsigned char>& pvchParentHash, unsigned int pnSequence, unsigned int pnDepth, unsigned short pnDerivationMethod)
{
    if (pvchChainCode.size() != 32) {
        throw keychain_error("CPrivChain::CPrivChain() : Invalid chain code.");
    }

    if (pvchKey.size() != 32 || !checkVector(pvchKey, true))
    {
        throw keychain_error("CPrivChain::CPrivChain() : Invalid private key.");
    }

    nDepth = pnDepth;
    nSequence = pnSequence;
    vchChainCode = pvchChainCode;
    vchParentHash = pvchParentHash;
    nDerivationMethod = pnDerivationMethod;
    vchPrivateKey = pvchKey;

    updatePublicKey();

    fValid = true;
}

CPrivChain& CPrivChain::operator=(const CPrivChain& pcRightOperand)
{
    fValid = pcRightOperand.fValid;

    if (fValid) {
        nDepth = pcRightOperand.nDepth;
        nSequence = pcRightOperand.nSequence;
        nDerivationMethod = pcRightOperand.nDerivationMethod;
        vchChainCode = pcRightOperand.vchChainCode;
        vchPrivateKey = pcRightOperand.vchPrivateKey;
        vchParentHash = pcRightOperand.vchParentHash;

        updatePublicKey();
    }

    return *this;
}

bool CPrivChain::operator==(const CPrivChain& pcRightOperand) const
{
    return (fValid && pcRightOperand.fValid &&
            nDepth == pcRightOperand.nDepth &&
            nSequence == pcRightOperand.nSequence &&
            vchChainCode == pcRightOperand.vchChainCode &&
            nDerivationMethod == pcRightOperand.nDerivationMethod &&
            vchPrivateKey == pcRightOperand.vchPrivateKey &&
            vchPublicKey == pcRightOperand.vchPublicKey &&
            vchParentHash == pcRightOperand.vchParentHash &&
            vchPublicKeyHash == pcRightOperand.vchPublicKeyHash);
}

bool CPrivChain::operator!=(const CPrivChain& pcRightOperand) const
{
    return !(*this == pcRightOperand);
}

CKey CPrivChain::getKey() const
{
    if (vchPrivateKey.size() != 32)
        throw keychain_error("CPrivChain::getKey() : you have to initialize object first.");

    CKey curvekey;
    curvekey.SetSecret(vchPrivateKey, true);

    return curvekey;
}

// New key pair derivation using supplied i, provides a basic 
//    CKD(m, i) and CKD'(m, i) implementation.
//
// NOTE: Public derivation is active by default. You can 
//    use private derivation for additional security, but this keys 
//    will be incompatible with CKD'(m, i).
//
// It's recommended to use private derivation for account nodes (i.e. nDepth = 1).
CPrivChain CPrivChain::getChild(unsigned int pnSequence) const
{
    if (!fValid) {
        throw keychain_error("CPrivChain::getChild() : CPrivChain is invalid.");
    }

    CPrivChain child;
    child.fValid = false;

    CSecret extended;

    if (isP(pnSequence))
        extended.assign(vchPrivateKey.begin(), vchPrivateKey.end());
    else
        extended.assign(vchPublicKey.begin(), vchPublicKey.end());

    // There are two derivation functions implemented:
    //
    //   Public derivation, CKD'(m, i);
    //   Private derivation, CKD(m, i).
    //
    // Public derivation of a new private key is almost 
    //   identical to process described in CPubChain::getChild.
    // Private derivation of a new private key is the similar process too,
    //   but it uses a private key to create LR vector.
    //
    // Private key is computed as Ki = ( L(i) + K(i-1) ) % CURVE_ORDER

    extended.push_back(pnSequence >> 24);
    extended.push_back((pnSequence >> 16) & 0xff);
    extended.push_back((pnSequence >>  8) & 0xff);
    extended.push_back(pnSequence & 0xff);

    // HMAC SHA-512 digest of public key, using right half of parent key digest as password.
    // Right half should be filled with zeros for root node generation.
    CSecret hmac(64);
    getDigest(vchChainCode, extended, hmac);

    // Entropy source for key derivation
    std::vector<unsigned char> left32(hmac.begin(), hmac.begin() + 32);

    // Entropy source for the future generations
    std::vector<unsigned char> right32(hmac.begin() + 32, hmac.end());

    if (!derivateSecret(vchPrivateKey, left32, child.vchPrivateKey)) return child;
    child.vchPrivateKey.resize(32);

    child.updatePublicKey();

    child.nDepth = nDepth + 1;
    child.nSequence = pnSequence;
    child.vchChainCode = right32;
    child.nDerivationMethod = nDerivationMethod;
    child.fValid = true;

    return child;
}

// Regenerate an internal copy of public key and its hash. That's 
//   necessary because we are using public key hash as an entropy 
//   source for child nodes.
void CPrivChain::updatePublicKey() {
    CKey curvekey;
    CPubKey curvepubkey;

    CSecret prvKey;
    prvKey.assign(vchPrivateKey.begin(), vchPrivateKey.end());

    if (curvekey.SetSecret(prvKey, true))
    {
        curvepubkey = curvekey.GetPubKey();
    }
    else {
        throw keychain_error("CPrivChain::updatePublicKey() : SetSecret failure.");
    }

    vchPublicKey = curvepubkey.Raw();
    vchPublicKeyHash = curvepubkey.GetID().getvch();
}

// Serialize object and encode it as base58 string
std::string CPrivChain::ToString() const
{
    std::vector<unsigned char> vchTmp;
    unsigned short nVersion = CPrivChain::getVersion();

    vchTmp.push_back((nVersion >> 8) & 0xff);
    vchTmp.push_back(nVersion & 0xff);

    vchTmp.push_back((nDerivationMethod >> 8) & 0xff);
    vchTmp.push_back(nDerivationMethod & 0xff);

    vchTmp.push_back(nDepth >> 24);
    vchTmp.push_back((nDepth >> 16) & 0xff);
    vchTmp.push_back((nDepth >> 8) & 0xff);
    vchTmp.push_back(nDepth & 0xff);

    vchTmp.push_back(nSequence >> 24);
    vchTmp.push_back((nSequence >> 16) & 0xff);
    vchTmp.push_back((nSequence >> 8 ) & 0xff);
    vchTmp.push_back(nSequence & 0xff);

    vchTmp.insert(vchTmp.end(), vchChainCode.begin(), vchChainCode.end());
    vchTmp.insert(vchTmp.end(), vchPrivateKey.begin(), vchPrivateKey.end());
    vchTmp.insert(vchTmp.end(), vchParentHash.begin(), vchParentHash.end());

    return EncodeBase58Check(vchTmp);
}


// Construct a public chain object instance using our current chain params
CPubChain CPrivChain::getPublic() const
{
    if (!fValid) {
        throw keychain_error("CPrivChain::getPublic() : CKeyChain is invalid.");
    }

    CPubChain pub;
    pub.fValid = fValid;
    pub.nDepth = nDepth;
    pub.nSequence = nSequence;
    pub.nDerivationMethod = nDerivationMethod;

    pub.vchChainCode = vchChainCode;
    pub.vchPublicKey = vchPublicKey;
    pub.vchParentHash = vchParentHash;
    pub.vchPublicKeyHash = vchPublicKeyHash;

    return pub;
}

// CKD(m, i) core function, allows us to derive a new ECDSA secret using pregenerated info:
//
// * pvchSecret - the private key to derive from, 32 bytes vector value;
// * pvchLeft32 - left half of HMAC-SHA512(entropy, data + suffix)
//      entropy is 256 bits chain code for parent key,
//      data is a public key for public derivation or private key for private derivation,
//      suffix is 32 bit value computed from a child number.
//
//   Returns false and empty vector when
//      invalid pvchSecret value supplied (zero or greater than defined by internal ECDSA limit).
bool CPrivChain::derivateSecret(const CSecret pvchSecret, const std::vector<unsigned char> pvchLeft32, CSecret& pvchChildSecret) const
{
    bool fSuccess = true; // Valid key

    BIGNUM* bnKey = BN_bin2bn(&pvchSecret[0], pvchSecret.size(), NULL);
    BIGNUM* bnLeft32 = BN_bin2bn(&pvchLeft32[0], pvchLeft32.size(), NULL);
    BIGNUM *bnCurveOrder = BN_bin2bn(&CURVE_ORDER[0], CURVE_ORDER.size(), NULL);
    BN_CTX* ctx = BN_CTX_new();

    if (!bnKey) { throw keychain_error("CPrivChain::derivateSecret() : bnKey allocation error."); }
    if (!bnLeft32) { throw keychain_error("CPrivChain::derivateSecret() : bnLeft32 allocation error."); }
    if (!bnCurveOrder) { throw keychain_error("CPrivChain::derivateSecret() : bnCurveOrder allocation error."); }

    if (!ctx) {
        BN_free(bnKey);
        BN_free(bnLeft32);
        BN_free(bnCurveOrder);

        throw keychain_error("CPrivChain::derivateSecret() : ctx allocation error.");
    }

    if (BN_cmp(bnKey, bnCurveOrder) >= 0 || BN_is_zero(bnKey)) {
        fSuccess = false; // Invalid key
    } else {
        // Calculate Kc = (L + Kp) % CURVE_ORDER
        if (!BN_add(bnKey, bnKey, bnLeft32)) { throw keychain_error("CPrivChain::derivateSecret() : BN_add error."); }
        if (!BN_div(NULL, bnKey, bnKey, bnCurveOrder, ctx)) throw keychain_error("CPrivChain::derivateSecret() : BN_div error.");

        if (BN_is_zero(bnKey))
            fSuccess = false; // Invalid key
    }

    if (fSuccess)
    {
        pvchChildSecret.resize(BN_num_bytes(bnKey));
        BN_bn2bin(bnKey, &pvchChildSecret[0]);
    }

    BN_CTX_free(ctx);
    BN_clear_free(bnKey);
    BN_clear_free(bnLeft32);
    BN_clear_free(bnCurveOrder);

    return fSuccess;
}
