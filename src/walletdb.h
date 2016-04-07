// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_WALLETDB_H
#define BITCOIN_WALLETDB_H

#include "db.h"
#include "keystore.h"

using namespace std;

class CKeyPool;
class CAccount;
class CAccountingEntry;

/** Error statuses for the wallet database */
enum DBErrors
{
    DB_LOAD_OK,
    DB_CORRUPT,
    DB_NONCRITICAL_ERROR,
    DB_TOO_NEW,
    DB_LOAD_FAIL,
    DB_NEED_REWRITE
};

class CKeyMetadata
{
public:
    static const int CURRENT_VERSION=1;
    int nVersion;
    int64_t nCreateTime; // 0 means unknown

    CKeyMetadata()
    {
        SetNull();
    }
    CKeyMetadata(int64_t nCreateTime_)
    {
        nVersion = CKeyMetadata::CURRENT_VERSION;
        nCreateTime = nCreateTime_;
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(nCreateTime);
    )

    void SetNull()
    {
        nVersion = CKeyMetadata::CURRENT_VERSION;
        nCreateTime = 0;
    }
};


/** Access to the wallet database (wallet.dat) */
class CWalletDB : public CDB
{
public:
    CWalletDB(string strFilename, const char* pszMode="r+") : CDB(strFilename.c_str(), pszMode)
    {
    }
private:
    CWalletDB(const CWalletDB&);
    void operator=(const CWalletDB&);
public:
    bool WriteName(const string& strAddress, const string& strName);

    bool EraseName(const string& strAddress);

    bool WriteTx(uint256 hash, const CWalletTx& wtx)
    {
        nWalletDBUpdated++;
        return Write(make_pair(string("tx"), hash), wtx);
    }

    bool EraseTx(uint256 hash)
    {
        nWalletDBUpdated++;
        return Erase(make_pair(string("tx"), hash));
    }

    bool WriteKey(const CPubKey& key, const CPrivKey& vchPrivKey, const CKeyMetadata &keyMeta)
    {
        nWalletDBUpdated++;
        if(!Write(make_pair(string("keymeta"), key), keyMeta))
            return false;

        if(!Write(make_pair(string("key"), key), vchPrivKey, false))
            return false;

        return true;
    }

    bool WriteMalleableKey(const CMalleableKeyView& keyView, const CSecret& vchSecretH, const CKeyMetadata &keyMeta)
    {
        nWalletDBUpdated++;
        if(!Write(make_pair(string("malmeta"), keyView.ToString()), keyMeta))
            return false;

        if(!Write(make_pair(string("malpair"), keyView.ToString()), vchSecretH, false))
            return false;

        return true;
    }

    bool WriteCryptedMalleableKey(const CMalleableKeyView& keyView, const vector<unsigned char>& vchCryptedSecretH, const CKeyMetadata &keyMeta)
    {
        nWalletDBUpdated++;
        if(!Write(make_pair(string("malmeta"), keyView.ToString()), keyMeta))
            return false;

        if(!Write(make_pair(string("malcpair"), keyView.ToString()), vchCryptedSecretH, false))
            return false;

        Erase(make_pair(string("malpair"), keyView.ToString()));

        return true;
    }


    bool WriteCryptedKey(const CPubKey& key, const vector<unsigned char>& vchCryptedSecret, const CKeyMetadata &keyMeta)
    {
        nWalletDBUpdated++;
        bool fEraseUnencryptedKey = true;

        if(!Write(make_pair(string("keymeta"), key), keyMeta))
            return false;

        if (!Write(make_pair(string("ckey"), key), vchCryptedSecret, false))
            return false;
        if (fEraseUnencryptedKey)
        {
            Erase(make_pair(string("key"), key));
            Erase(make_pair(string("wkey"), key));
        }
        return true;
    }

    bool WriteMasterKey(unsigned int nID, const CMasterKey& kMasterKey)
    {
        nWalletDBUpdated++;
        return Write(make_pair(string("mkey"), nID), kMasterKey, true);
    }

    bool EraseMasterKey(unsigned int nID)
    {
        nWalletDBUpdated++;
        return Erase(make_pair(string("mkey"), nID));
    }

    bool EraseCryptedKey(const CPubKey& key)
    {
        return Erase(make_pair(string("ckey"), key));
    }

    bool EraseCryptedMalleableKey(const CMalleableKeyView& keyView)
    {
        return Erase(make_pair(string("malcpair"), keyView.ToString()));
    }

    bool WriteCScript(const uint160& hash, const CScript& redeemScript)
    {
        nWalletDBUpdated++;
        return Write(make_pair(string("cscript"), hash), redeemScript, false);
    }

    bool WriteWatchOnly(const CScript &dest)
    {
        nWalletDBUpdated++;
        return Write(make_pair(string("watchs"), dest), '1');
    }

    bool EraseWatchOnly(const CScript &dest)
    {
        nWalletDBUpdated++;
        return Erase(make_pair(string("watchs"), dest));
    }

    bool WriteBestBlock(const CBlockLocator& locator)
    {
        nWalletDBUpdated++;
        return Write(string("bestblock"), locator);
    }

    bool ReadBestBlock(CBlockLocator& locator)
    {
        return Read(string("bestblock"), locator);
    }

    bool WriteOrderPosNext(int64_t nOrderPosNext)
    {
        nWalletDBUpdated++;
        return Write(string("orderposnext"), nOrderPosNext);
    }

    bool WriteDefaultKey(const CPubKey& key)
    {
        nWalletDBUpdated++;
        return Write(string("defaultkey"), key);
    }

    bool ReadPool(int64_t nPool, CKeyPool& keypool)
    {
        return Read(make_pair(string("pool"), nPool), keypool);
    }

    bool WritePool(int64_t nPool, const CKeyPool& keypool)
    {
        nWalletDBUpdated++;
        return Write(make_pair(string("pool"), nPool), keypool);
    }

    bool ErasePool(int64_t nPool)
    {
        nWalletDBUpdated++;
        return Erase(make_pair(string("pool"), nPool));
    }

    bool WriteMinVersion(int nVersion)
    {
        return Write(string("minversion"), nVersion);
    }

    bool ReadAccount(const string& strAccount, CAccount& account);
    bool WriteAccount(const string& strAccount, const CAccount& account);
private:
    bool WriteAccountingEntry(const uint64_t nAccEntryNum, const CAccountingEntry& acentry);
public:
    bool WriteAccountingEntry(const CAccountingEntry& acentry);
    int64_t GetAccountCreditDebit(const string& strAccount);
    void ListAccountCreditDebit(const string& strAccount, list<CAccountingEntry>& acentries);

    DBErrors ReorderTransactions(CWallet*);
    DBErrors LoadWallet(CWallet* pwallet);
    DBErrors FindWalletTx(CWallet* pwallet, vector<uint256>& vTxHash);
    DBErrors ZapWalletTx(CWallet* pwallet);

    static bool Recover(CDBEnv& dbenv, string filename, bool fOnlyKeys);
    static bool Recover(CDBEnv& dbenv, string filename);
};

#endif // BITCOIN_WALLETDB_H
