// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_KEYSTORE_H
#define BITCOIN_KEYSTORE_H

#include "crypter.h"
#include "sync.h"
#include <boost/signals2/signal.hpp>
#include <boost/variant.hpp>

class CScript;

class CNoDestination {
public:
    friend bool operator==(const CNoDestination &a, const CNoDestination &b) { return true; }
    friend bool operator<(const CNoDestination &a, const CNoDestination &b) { return true; }
};

/** A txout script template with a specific destination. It is either:
  * CNoDestination: no destination set
  * CKeyID: TX_PUBKEYHASH destination
  * CScriptID: TX_SCRIPTHASH destination
  *
  * A CTxDestination is the internal data type encoded in a CBitcoinAddress.
  */
typedef boost::variant<CNoDestination, CKeyID, CScriptID> CTxDestination;

/** A virtual base class for key stores */
class CKeyStore
{
protected:
    mutable CCriticalSection cs_KeyStore;

public:
    virtual ~CKeyStore() {}

    // Add a key to the store.
    virtual bool AddKey(const CKey& key) =0;

    // Add a malleable key to store.
    virtual bool AddMalleableKey(const CMalleableKeyView &keyView, const CSecret &vchSecretH) =0;
    virtual bool GetMalleableKey(const CMalleableKeyView &keyView, CMalleableKey &mKey) const =0;

    // Check whether a key corresponding to a given address is present in the store.
    virtual bool HaveKey(const CKeyID &address) const =0;
    virtual bool GetKey(const CKeyID &address, CKey& keyOut) const =0;
    virtual void GetKeys(std::set<CKeyID> &setAddress) const =0;
    virtual bool GetPubKey(const CKeyID &address, CPubKey& vchPubKeyOut) const;

    // Support for BIP 0013 : see https://en.bitcoin.it/wiki/BIP_0013
    virtual bool AddCScript(const CScript& redeemScript) =0;
    virtual bool HaveCScript(const CScriptID &hash) const =0;
    virtual bool GetCScript(const CScriptID &hash, CScript& redeemScriptOut) const =0;

    // Support for Watch-only addresses
    virtual bool AddWatchOnly(const CScript &dest) =0;
    virtual bool RemoveWatchOnly(const CScript &dest) =0;
    virtual bool HaveWatchOnly(const CScript &dest) const =0;
    virtual bool HaveWatchOnly() const =0;

    virtual bool GetSecret(const CKeyID &address, CSecret& vchSecret, bool &fCompressed) const;

    virtual bool CheckOwnership(const CPubKey &pubKeyVariant, const CPubKey &R) const =0;
    virtual bool CheckOwnership(const CPubKey &pubKeyVariant, const CPubKey &R, CMalleableKeyView &view) const =0;
    virtual bool CreatePrivKey(const CPubKey &pubKeyVariant, const CPubKey &R, CKey &privKey) const =0;
    virtual void ListMalleableViews(std::list<CMalleableKeyView> &malleableViewList) const =0;
};

typedef std::map<CKeyID, std::pair<CSecret, bool> > KeyMap;
typedef std::map<CScriptID, CScript > ScriptMap;
typedef std::set<CScript> WatchOnlySet;
typedef std::map<CMalleableKeyView, CSecret> MalleableKeyMap;

/** Basic key store, that keeps keys in an address->secret map */
class CBasicKeyStore : public CKeyStore
{
protected:
    KeyMap mapKeys;
    MalleableKeyMap mapMalleableKeys;

    ScriptMap mapScripts;
    WatchOnlySet setWatchOnly;

public:
    bool AddKey(const CKey& key);
    bool AddMalleableKey(const CMalleableKeyView& keyView, const CSecret &vchSecretH);
    bool GetMalleableKey(const CMalleableKeyView &keyView, CMalleableKey &mKey) const;
    bool HaveKey(const CKeyID &address) const;
    void GetKeys(std::set<CKeyID> &setAddress) const;
    bool GetKey(const CKeyID &address, CKey &keyOut) const;
    virtual bool AddCScript(const CScript& redeemScript);
    virtual bool HaveCScript(const CScriptID &hash) const;
    virtual bool GetCScript(const CScriptID &hash, CScript& redeemScriptOut) const;

    virtual bool AddWatchOnly(const CScript &dest);
    virtual bool RemoveWatchOnly(const CScript &dest);
    virtual bool HaveWatchOnly(const CScript &dest) const;
    virtual bool HaveWatchOnly() const;

    bool CheckOwnership(const CPubKey &pubKeyVariant, const CPubKey &R) const;
    bool CheckOwnership(const CPubKey &pubKeyVariant, const CPubKey &R, CMalleableKeyView &view) const;
    bool CreatePrivKey(const CPubKey &pubKeyVariant, const CPubKey &R, CKey &privKey) const;
    void ListMalleableViews(std::list<CMalleableKeyView> &malleableViewList) const;
    bool GetMalleableView(const CMalleablePubKey &mpk, CMalleableKeyView &view);
};

typedef std::map<CKeyID, std::pair<CPubKey, std::vector<unsigned char> > > CryptedKeyMap;
typedef std::map<CMalleableKeyView, std::vector<unsigned char> > CryptedMalleableKeyMap;

/** Keystore which keeps the private keys encrypted.
 * It derives from the basic key store, which is used if no encryption is active.
 */
class CCryptoKeyStore : public CBasicKeyStore
{
private:
    CryptedKeyMap mapCryptedKeys;
    CryptedMalleableKeyMap mapCryptedMalleableKeys;

    CKeyingMaterial vMasterKey;

    // if fUseCrypto is true, mapKeys must be empty
    // if fUseCrypto is false, vMasterKey must be empty
    bool fUseCrypto;

    // keeps track of whether Unlock has run a thorough check before
    bool fDecryptionThoroughlyChecked;

protected:
    bool SetCrypted();

    // will encrypt previously unencrypted keys
    bool EncryptKeys(CKeyingMaterial& vMasterKeyIn);
    bool DecryptKeys(const CKeyingMaterial& vMasterKeyIn);

    bool Unlock(const CKeyingMaterial& vMasterKeyIn);

public:
    CCryptoKeyStore();

    bool IsCrypted() const;
    bool IsLocked() const;
    bool Lock();

    virtual bool AddCryptedKey(const CPubKey &vchPubKey, const std::vector<unsigned char> &vchCryptedSecret);
    virtual bool AddCryptedMalleableKey(const CMalleableKeyView& keyView, const std::vector<unsigned char>  &vchCryptedSecretH);

    bool AddKey(const CKey& key);
    bool AddMalleableKey(const CMalleableKeyView& keyView, const CSecret &vchSecretH);
    bool HaveKey(const CKeyID &address) const;
    bool GetKey(const CKeyID &address, CKey& keyOut) const;
    bool GetPubKey(const CKeyID &address, CPubKey& vchPubKeyOut) const;
    void GetKeys(std::set<CKeyID> &setAddress) const;
    bool GetMalleableKey(const CMalleableKeyView &keyView, CMalleableKey &mKey) const;
    bool CheckOwnership(const CPubKey &pubKeyVariant, const CPubKey &R) const;
    bool CheckOwnership(const CPubKey &pubKeyVariant, const CPubKey &R, CMalleableKeyView &view) const;
    bool CheckOwnership(const CMalleablePubKey &mpk);
    bool CreatePrivKey(const CPubKey &pubKeyVariant, const CPubKey &R, CKey &privKey) const;
    void ListMalleableViews(std::list<CMalleableKeyView> &malleableViewList) const;
    bool GetMalleableView(const CMalleablePubKey &mpk, CMalleableKeyView &view);
    /* Wallet status (encrypted, locked) changed.
     * Note: Called without locks held.
     */
    boost::signals2::signal<void (CCryptoKeyStore* wallet)> NotifyStatusChanged;
};

#endif
