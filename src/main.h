// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_MAIN_H
#define BITCOIN_MAIN_H

#include "timestamps.h"
#include "bignum.h"
#include "sync.h"
#include "net.h"
#include "script.h"
#include "scrypt.h"
#include "zerocoin/Zerocoin.h"

#include <list>

class CWallet;
class CBlock;
class CBlockIndex;
class CKeyItem;
class CReserveKey;
class COutPoint;

class CAddress;
class CInv;
class CRequestTracker;
class CNode;
class CBlockIndexTrustComparator;

static const unsigned int MAX_BLOCK_SIZE = 1000000;
static const unsigned int MAX_BLOCK_SIZE_GEN = MAX_BLOCK_SIZE/2;
static const unsigned int MAX_BLOCK_SIGOPS = MAX_BLOCK_SIZE/50;
static const unsigned int MAX_ORPHAN_TRANSACTIONS = MAX_BLOCK_SIZE/100;
static const unsigned int MAX_INV_SZ = 50000;

static const unsigned int MAX_BLOCKFILE_SIZE = 0x8000000; // 128 MiB
static const unsigned int BLOCKFILE_CHUNK_SIZE = 0x1000000; // 16 MiB
static const unsigned int UNDOFILE_CHUNK_SIZE = 0x100000; // 1 MiB
static const unsigned int MEMPOOL_HEIGHT = 0x7FFFFFFF;

static const int64 MIN_TX_FEE = CENT / 10;
static const int64 MIN_RELAY_TX_FEE = CENT / 50;

static const int64 MAX_MONEY = 2000000000 * COIN;
static const int64 MAX_MINT_PROOF_OF_WORK = 100 * COIN;
static const int64 MAX_MINT_PROOF_OF_STAKE = 1 * COIN;
static const int64 MIN_TXOUT_AMOUNT = CENT / 100;

inline bool MoneyRange(int64 nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }
// Threshold for nLockTime: below this value it is interpreted as block number, otherwise as UNIX timestamp.
static const unsigned int LOCKTIME_THRESHOLD = 500000000; // Tue Nov  5 00:53:20 1985 UTC

#ifdef USE_UPNP
static const int fHaveUPnP = true;
#else
static const int fHaveUPnP = false;
#endif

static const uint256 hashGenesisBlock("0x00000a060336cbb72fe969666d337b87198b1add2abaa59cca226820b32933a4");
static const uint256 hashGenesisBlockTestNet("0x000c763e402f2436da9ed36c7286f62c3f6e5dbafce9ff289bd43d7459327eb");

inline int64 PastDrift(int64 nTime)   { return nTime - 2 * 60 * 60; } // up to 2 hours from the past
inline int64 FutureDrift(int64 nTime) { return nTime + 2 * 60 * 60; } // up to 2 hours from the future

extern libzerocoin::Params* ZCParams;
extern CScript COINBASE_FLAGS;
extern CCriticalSection cs_main;
extern std::map<uint256, CBlockIndex*> mapBlockIndex;
extern std::set<CBlockIndex*, CBlockIndexTrustComparator> setBlockIndexValid;
extern std::set<std::pair<COutPoint, unsigned int> > setStakeSeen;
extern CBlockIndex* pindexGenesisBlock;
extern unsigned int nStakeMinAge;
extern unsigned int nNodeLifespan;
extern int nCoinbaseMaturity;
extern int nBestHeight;
extern uint256 nBestChainTrust;
extern uint256 nBestInvalidTrust;
extern uint256 hashBestChain;
extern CBlockIndex* pindexBest;
extern unsigned int nTransactionsUpdated;
extern uint64 nLastBlockTx;
extern uint64 nLastBlockSize;
extern int64 nLastCoinStakeSearchInterval;
extern const std::string strMessageMagic;
extern int64 nTimeBestReceived;
extern CCriticalSection cs_setpwalletRegistered;
extern std::set<CWallet*> setpwalletRegistered;
extern unsigned char pchMessageStart[4];
extern std::map<uint256, CBlock*> mapOrphanBlocks;

// Settings
extern int64 nTransactionFee;
extern int64 nMinimumInputValue;
extern bool fUseFastIndex;
extern unsigned int nDerivationMethodIndex;
extern bool fEnforceCanonical;

// Minimum disk space required - used in CheckDiskSpace()
static const uint64 nMinDiskSpace = 52428800;

class CReserveKey;
class CCoinsDB;
class CBlockTreeDB;
class CDiskBlockPos;
class CCoins;
class CTxUndo;
class CCoinsView;
class CCoinsViewCache;

void RegisterWallet(CWallet* pwalletIn);
void UnregisterWallet(CWallet* pwalletIn);
void SyncWithWallets(const uint256 &hash, const CTransaction& tx, const CBlock* pblock = NULL, bool fUpdate = false, bool fConnect = true);
bool ProcessBlock(CNode* pfrom, CBlock* pblock);
bool CheckDiskSpace(uint64 nAdditionalBytes=0);
FILE* OpenBlockFile(const CDiskBlockPos &pos, bool fReadOnly = false);
FILE* OpenUndoFile(const CDiskBlockPos &pos, bool fReadOnly = false);
bool LoadBlockIndex(bool fAllowNew=true);
void PrintBlockTree();
CBlockIndex* FindBlockByHeight(int nHeight);
bool ProcessMessages(CNode* pfrom);
bool SendMessages(CNode* pto, bool fSendTrickle);
bool LoadExternalBlockFile(FILE* fileIn);

bool CheckProofOfWork(uint256 hash, unsigned int nBits);
unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake);
int64 GetProofOfWorkReward(unsigned int nBits, int64 nFees=0);
int64 GetProofOfStakeReward(int64 nCoinAge, unsigned int nBits, unsigned int nTime, bool bCoinYearOnly=false);
unsigned int ComputeMinWork(unsigned int nBase, int64 nTime);
unsigned int ComputeMinStake(unsigned int nBase, int64 nTime, unsigned int nBlockTime);
int GetNumBlocksOfPeers();
bool IsInitialBlockDownload();
std::string GetWarnings(std::string strFor);
bool GetTransaction(const uint256 &hash, CTransaction &tx, uint256 &hashBlock, bool fAllowSlow);
bool SetBestChain(CBlockIndex* pindexNew);
bool ConnectBestBlock();
CBlockIndex * InsertBlockIndex(uint256 hash);
uint256 WantedByOrphan(const CBlock* pblockOrphan);
const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake);
void StakeMiner(CWallet *pwallet);
void ResendWalletTransactions(bool fForce=false);

bool GetWalletFile(CWallet* pwallet, std::string &strWalletFileOut);

class CDiskBlockPos
{
public:
    int nFile;
    unsigned int nPos;

    IMPLEMENT_SERIALIZE(
        READWRITE(VARINT(nFile));
        READWRITE(VARINT(nPos));
    )

    friend bool operator==(const CDiskBlockPos &a, const CDiskBlockPos &b) {
        return (a.nFile == b.nFile && a.nPos == b.nPos);
    }

    friend bool operator!=(const CDiskBlockPos &a, const CDiskBlockPos &b) {
        return !(a == b);
    }

    void SetNull() { nFile = -1; nPos = 0; }
    bool IsNull() const { return (nFile == -1); }
};


/** An inpoint - a combination of a transaction and an index n into its vin */
class CInPoint
{
public:
    CTransaction* ptx;
    unsigned int n;

    CInPoint() { SetNull(); }
    CInPoint(CTransaction* ptxIn, unsigned int nIn) { ptx = ptxIn; n = nIn; }
    void SetNull() { ptx = NULL; n = (unsigned int) -1; }
    bool IsNull() const { return (ptx == NULL && n == (unsigned int) -1); }
};



/** An outpoint - a combination of a transaction hash and an index n into its vout */
class COutPoint
{
public:
    uint256 hash;
    unsigned int n;

    COutPoint() { SetNull(); }
    COutPoint(uint256 hashIn, unsigned int nIn) { hash = hashIn; n = nIn; }
    IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
    void SetNull() { hash = 0; n = (unsigned int) -1; }
    bool IsNull() const { return (hash == 0 && n == (unsigned int) -1); }

    friend bool operator<(const COutPoint& a, const COutPoint& b)
    {
        return (a.hash < b.hash || (a.hash == b.hash && a.n < b.n));
    }

    friend bool operator==(const COutPoint& a, const COutPoint& b)
    {
        return (a.hash == b.hash && a.n == b.n);
    }

    friend bool operator!=(const COutPoint& a, const COutPoint& b)
    {
        return !(a == b);
    }

    std::string ToString() const
    {
        return strprintf("COutPoint(%s, %u)", hash.ToString().substr(0,10).c_str(), n);
    }

    void print() const
    {
        printf("%s\n", ToString().c_str());
    }
};




/** An input of a transaction.  It contains the location of the previous
 * transaction's output that it claims and a signature that matches the
 * output's public key.
 */
class CTxIn
{
public:
    COutPoint prevout;
    CScript scriptSig;
    unsigned int nSequence;

    CTxIn()
    {
        nSequence = std::numeric_limits<unsigned int>::max();
    }

    explicit CTxIn(COutPoint prevoutIn, CScript scriptSigIn=CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max())
    {
        prevout = prevoutIn;
        scriptSig = scriptSigIn;
        nSequence = nSequenceIn;
    }

    CTxIn(uint256 hashPrevTx, unsigned int nOut, CScript scriptSigIn=CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max())
    {
        prevout = COutPoint(hashPrevTx, nOut);
        scriptSig = scriptSigIn;
        nSequence = nSequenceIn;
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(prevout);
        READWRITE(scriptSig);
        READWRITE(nSequence);
    )

    bool IsFinal() const
    {
        return (nSequence == std::numeric_limits<unsigned int>::max());
    }

    friend bool operator==(const CTxIn& a, const CTxIn& b)
    {
        return (a.prevout   == b.prevout &&
                a.scriptSig == b.scriptSig &&
                a.nSequence == b.nSequence);
    }

    friend bool operator!=(const CTxIn& a, const CTxIn& b)
    {
        return !(a == b);
    }

    std::string ToStringShort() const
    {
        return strprintf(" %s %d", prevout.hash.ToString().c_str(), prevout.n);
    }

    std::string ToString() const
    {
        std::string str;
        str += "CTxIn(";
        str += prevout.ToString();
        if (prevout.IsNull())
            str += strprintf(", coinbase %s", HexStr(scriptSig).c_str());
        else
            str += strprintf(", scriptSig=%s", scriptSig.ToString().substr(0,24).c_str());
        if (nSequence != std::numeric_limits<unsigned int>::max())
            str += strprintf(", nSequence=%u", nSequence);
        str += ")";
        return str;
    }

    void print() const
    {
        printf("%s\n", ToString().c_str());
    }
};




/** An output of a transaction.  It contains the public key that the next input
 * must be able to sign with to claim it.
 */
class CTxOut
{
public:
    int64 nValue;
    CScript scriptPubKey;

    CTxOut()
    {
        SetNull();
    }

    CTxOut(int64 nValueIn, CScript scriptPubKeyIn)
    {
        nValue = nValueIn;
        scriptPubKey = scriptPubKeyIn;
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(nValue);
        READWRITE(scriptPubKey);
    )

    void SetNull()
    {
        nValue = -1;
        scriptPubKey.clear();
    }

    bool IsNull() const
    {
        return (nValue == -1);
    }

    void SetEmpty()
    {
        nValue = 0;
        scriptPubKey.clear();
    }

    bool IsEmpty() const
    {
        return (nValue == 0 && scriptPubKey.empty());
    }

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }

    friend bool operator==(const CTxOut& a, const CTxOut& b)
    {
        return (a.nValue       == b.nValue &&
                a.scriptPubKey == b.scriptPubKey);
    }

    friend bool operator!=(const CTxOut& a, const CTxOut& b)
    {
        return !(a == b);
    }

    std::string ToStringShort() const
    {
        return strprintf(" out %s %s", FormatMoney(nValue).c_str(), scriptPubKey.ToString().substr(0, 10).c_str());
    }

    std::string ToString() const
    {
        if (IsEmpty()) return "CTxOut(empty)";
        if (scriptPubKey.size() < 6)
            return "CTxOut(error)";
        return strprintf("CTxOut(nValue=%s, scriptPubKey=%s)", FormatMoney(nValue).c_str(), scriptPubKey.ToString().c_str());
    }

    void print() const
    {
        printf("%s\n", ToString().c_str());
    }
};




enum GetMinFee_mode
{
    GMF_BLOCK,
    GMF_RELAY,
    GMF_SEND,
};

// Modes for script/signature checking
enum CheckSig_mode
{
    CS_NEVER,             // never validate scripts
    CS_AFTER_CHECKPOINT,  // validate scripts after the last checkpoint
    CS_ALWAYS             // always validate scripts
};

/** The basic transaction that is broadcasted on the network and contained in
 * blocks.  A transaction can contain multiple inputs and outputs.
 */
class CTransaction
{
public:
    static const int CURRENT_VERSION=1;
    int nVersion;
    unsigned int nTime;
    std::vector<CTxIn> vin;
    std::vector<CTxOut> vout;
    unsigned int nLockTime;

    // Denial-of-service detection:
    mutable int nDoS;
    bool DoS(int nDoSIn, bool fIn) const { nDoS += nDoSIn; return fIn; }

    CTransaction()
    {
        SetNull();
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(nTime);
        READWRITE(vin);
        READWRITE(vout);
        READWRITE(nLockTime);
    )

    void SetNull()
    {
        nVersion = CTransaction::CURRENT_VERSION;
        nTime = GetAdjustedTime();
        vin.clear();
        vout.clear();
        nLockTime = 0;
        nDoS = 0;  // Denial-of-service prevention
    }

    bool IsNull() const
    {
        return (vin.empty() && vout.empty());
    }

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }

    uint256 GetMetaHash() const
    {
        return SignatureHash(CScript(), *this, 0, SIGHASH_ALL);
    }

    bool IsFinal(int nBlockHeight=0, int64 nBlockTime=0) const
    {
        // Time based nLockTime implemented in 0.1.6
        if (nLockTime == 0)
            return true;
        if (nBlockHeight == 0)
            nBlockHeight = nBestHeight;
        if (nBlockTime == 0)
            nBlockTime = GetAdjustedTime();
        if ((int64)nLockTime < ((int64)nLockTime < LOCKTIME_THRESHOLD ? (int64)nBlockHeight : nBlockTime))
            return true;
        BOOST_FOREACH(const CTxIn& txin, vin)
            if (!txin.IsFinal())
                return false;
        return true;
    }

    bool IsNewerThan(const CTransaction& old) const
    {
        if (vin.size() != old.vin.size())
            return false;
        for (unsigned int i = 0; i < vin.size(); i++)
            if (vin[i].prevout != old.vin[i].prevout)
                return false;

        bool fNewer = false;
        unsigned int nLowest = std::numeric_limits<unsigned int>::max();
        for (unsigned int i = 0; i < vin.size(); i++)
        {
            if (vin[i].nSequence != old.vin[i].nSequence)
            {
                if (vin[i].nSequence <= nLowest)
                {
                    fNewer = false;
                    nLowest = vin[i].nSequence;
                }
                if (old.vin[i].nSequence < nLowest)
                {
                    fNewer = true;
                    nLowest = old.vin[i].nSequence;
                }
            }
        }
        return fNewer;
    }

    bool IsCoinBase() const
    {
        return (vin.size() == 1 && vin[0].prevout.IsNull() && vout.size() >= 1);
    }

    bool IsCoinStake() const
    {
        // ppcoin: the coin stake transaction is marked with the first output empty
        return (vin.size() > 0 && (!vin[0].prevout.IsNull()) && vout.size() >= 2 && vout[0].IsEmpty());
    }

    /** Check for standard transaction types
        @return True if all outputs (scriptPubKeys) use only standard transaction forms
    */
    bool IsStandard() const;

    /** Check for standard transaction types
        @param[in] mapInputs	Map of previous transactions that have outputs we're spending
        @return True if all inputs (scriptSigs) use only standard transaction forms
        @see CTransaction::FetchInputs
    */
    bool AreInputsStandard(CCoinsViewCache& mapInputs) const;

    /** Count ECDSA signature operations the old-fashioned (pre-0.6) way
        @return number of sigops this transaction's outputs will produce when spent
        @see CTransaction::FetchInputs
    */
    unsigned int GetLegacySigOpCount() const;

    /** Count ECDSA signature operations in pay-to-script-hash inputs.

        @param[in] mapInputs	Map of previous transactions that have outputs we're spending
        @return maximum number of sigops required to validate this transaction's inputs
        @see CTransaction::FetchInputs
     */
    unsigned int GetP2SHSigOpCount(CCoinsViewCache& mapInputs) const;

    /** Amount of bitcoins spent by this transaction.
        @return sum of all outputs (note: does not include fees)
     */
    int64 GetValueOut() const
    {
        int64 nValueOut = 0;
        BOOST_FOREACH(const CTxOut& txout, vout)
        {
            nValueOut += txout.nValue;
            if (!MoneyRange(txout.nValue) || !MoneyRange(nValueOut))
                throw std::runtime_error("CTransaction::GetValueOut() : value out of range");
        }
        return nValueOut;
    }

    /** Amount of bitcoins coming in to this transaction
        Note that lightweight clients may not know anything besides the hash of previous transactions,
        so may not be able to calculate this.

        @param[in] mapInputs	Map of previous transactions that have outputs we're spending
        @return	Sum of value of all inputs (scriptSigs)
        @see CTransaction::FetchInputs
     */
    int64 GetValueIn(CCoinsViewCache& mapInputs) const;

    static bool AllowFree(double dPriority)
    {
        // Large (in bytes) low-priority (new, small-coin) transactions
        // need a fee.
        return dPriority > COIN * 144 / 250;
    }

    int64 GetMinFee(unsigned int nBlockSize=1, bool fAllowFree=false, enum GetMinFee_mode mode=GMF_BLOCK, unsigned int nBytes = 0) const;

    friend bool operator==(const CTransaction& a, const CTransaction& b)
    {
        return (a.nVersion  == b.nVersion &&
                a.nTime     == b.nTime &&
                a.vin       == b.vin &&
                a.vout      == b.vout &&
                a.nLockTime == b.nLockTime);
    }

    friend bool operator!=(const CTransaction& a, const CTransaction& b)
    {
        return !(a == b);
    }

    std::string ToStringShort() const
    {
        std::string str;
        str += strprintf("%s %s", GetHash().ToString().c_str(), IsCoinBase()? "base" : (IsCoinStake()? "stake" : "user"));
        return str;
    }

    std::string ToString() const
    {
        std::string str;
        str += IsCoinBase()? "Coinbase" : (IsCoinStake()? "Coinstake" : "CTransaction");
        str += strprintf("(hash=%s, nTime=%d, ver=%d, vin.size=%"PRIszu", vout.size=%"PRIszu", nLockTime=%d)\n",
            GetHash().ToString().substr(0,10).c_str(),
            nTime,
            nVersion,
            vin.size(),
            vout.size(),
            nLockTime);
        for (unsigned int i = 0; i < vin.size(); i++)
            str += "    " + vin[i].ToString() + "\n";
        for (unsigned int i = 0; i < vout.size(); i++)
            str += "    " + vout[i].ToString() + "\n";
        return str;
    }

    void print() const
    {
        printf("%s", ToString().c_str());
    }


    // Do all possible client-mode checks
    bool ClientCheckInputs() const;

    // Check whether all prevouts of this transaction are present in the UTXO set represented by view
    bool HaveInputs(CCoinsViewCache &view) const;

    // Check whether all inputs of this transaction are valid (no double spends, scripts & sigs, amounts)
    // This does not modify the UTXO set
    bool CheckInputs(CCoinsViewCache &view, enum CheckSig_mode csmode, bool fStrictPayToScriptHash=true, bool fStrictEncodings=true, CBlock *pblock=NULL) const;

    // Apply the effects of this transaction on the UTXO set represented by view
    bool UpdateCoins(CCoinsViewCache &view, CTxUndo &txundo, int nHeight, unsigned int nBlockTime, const uint256 &txhash) const;

    // Context-independent validity checks
    bool CheckTransaction() const;

    // Try to accept this transaction into the memory pool
    bool AcceptToMemoryPool(bool fCheckInputs=true, bool* pfMissingInputs=NULL);
    bool GetCoinAge(uint64& nCoinAge) const;  // Get transaction coin age
protected:
    static const CTxOut &GetOutputFor(const CTxIn& input, CCoinsViewCache& mapInputs);
};


/** wrapper for CTxOut that provides a more compact serialization */
class CTxOutCompressor
{
private:
    CTxOut &txout;

public:
    static uint64 CompressAmount(uint64 nAmount);
    static uint64 DecompressAmount(uint64 nAmount);

    CTxOutCompressor(CTxOut &txoutIn) : txout(txoutIn) { }

    IMPLEMENT_SERIALIZE(({
        if (!fRead) {
            uint64 nVal = CompressAmount(txout.nValue);
            READWRITE(VARINT(nVal));
        } else {
            uint64 nVal = 0;
            READWRITE(VARINT(nVal));
            txout.nValue = DecompressAmount(nVal);
        }
        CScriptCompressor cscript(REF(txout.scriptPubKey));
        READWRITE(cscript);
    });)
};

/** Undo information for a CTxIn
 *
 *  Contains the prevout's CTxOut being spent, and if this was the
 *  last output of the affected transaction, its metadata as well
 *  (coinbase or not, height, transaction version)
 */
class CTxInUndo
{
public:
    CTxOut txout;              // the txout data before being spent
    bool fCoinBase;            // if the outpoint was the last unspent: whether it belonged to a coinbase
    bool fCoinStake;           // if the outpoint was the last unspent: whether it belonged to a coinstake
    unsigned int nHeight;      // if the outpoint was the last unspent: its height
    int nVersion;              // if the outpoint was the last unspent: its version
    unsigned int nTime;        // if the outpoint was the last unspent: its timestamp
    unsigned int nBlockTime;   // if the outpoint was the last unspent: its block timestamp

    CTxInUndo() : txout(), fCoinBase(false), fCoinStake(false), nHeight(0), nVersion(0), nTime(0), nBlockTime(0) {}
    CTxInUndo(const CTxOut &txoutIn, bool fCoinBaseIn = false, bool fCoinStakeIn = false, unsigned int nHeightIn = 0, int nVersionIn = 0, int nTimeIn = 0, int nBlockTimeIn = 0) : txout(txoutIn), fCoinBase(fCoinBaseIn), fCoinStake(fCoinStakeIn), nHeight(nHeightIn), nVersion(nVersionIn), nTime(nTimeIn), nBlockTime(nBlockTimeIn) { }

    unsigned int GetSerializeSize(int nType, int nVersion) const {
        return ::GetSerializeSize(VARINT(nHeight*2+(fCoinBase ? 1 : 0)), nType, nVersion) +
               ::GetSerializeSize(VARINT(nTime*2+(fCoinStake ? 1 : 0)), nType, nVersion) +
               ::GetSerializeSize(VARINT(nBlockTime), nType, nVersion) +
               (nHeight > 0 ? ::GetSerializeSize(VARINT(this->nVersion), nType, nVersion) : 0) +
               ::GetSerializeSize(CTxOutCompressor(REF(txout)), nType, nVersion);
    }

    template<typename Stream>
    void Serialize(Stream &s, int nType, int nVersion) const {
        ::Serialize(s, VARINT(nHeight*2+(fCoinBase ? 1 : 0)), nType, nVersion);
        ::Serialize(s, VARINT(nTime*2+(fCoinStake ? 1 : 0)), nType, nVersion);
        ::Serialize(s, VARINT(nBlockTime), nType, nVersion);
        if (nHeight > 0)
            ::Serialize(s, VARINT(this->nVersion), nType, nVersion);
        ::Serialize(s, CTxOutCompressor(REF(txout)), nType, nVersion);
    }

    template<typename Stream>
    void Unserialize(Stream &s, int nType, int nVersion) {
        unsigned int nCodeHeight = 0, nCodeTime = 0;
        ::Unserialize(s, VARINT(nCodeHeight), nType, nVersion);
        nHeight = nCodeHeight / 2;
        fCoinBase = nCodeHeight & 1;
        ::Unserialize(s, VARINT(nCodeTime), nType, nVersion);
        nTime = nCodeTime / 2;
        fCoinStake = nCodeTime & 1;
        ::Unserialize(s, VARINT(nBlockTime), nType, nVersion);
        if (nHeight > 0)
            ::Unserialize(s, VARINT(this->nVersion), nType, nVersion);
        ::Unserialize(s, REF(CTxOutCompressor(REF(txout))), nType, nVersion);
    }
};

/** Undo information for a CTransaction */
class CTxUndo
{
public:
    // undo information for all txins
    std::vector<CTxInUndo> vprevout;

    IMPLEMENT_SERIALIZE(
        READWRITE(vprevout);
    )
};

/** Undo information for a CBlock */
class CBlockUndo
{
public:
    std::vector<CTxUndo> vtxundo; // for all but the coinbase

    IMPLEMENT_SERIALIZE(
        READWRITE(vtxundo);
    )

    bool WriteToDisk(CDiskBlockPos &pos)
    {
        // Open history file to append
        CAutoFile fileout = CAutoFile(OpenUndoFile(pos), SER_DISK, CLIENT_VERSION);
        if (!fileout)
            return error("CBlockUndo::WriteToDisk() : OpenUndoFile failed");

        // Write index header
        unsigned int nSize = fileout.GetSerializeSize(*this);
        fileout << FLATDATA(pchMessageStart) << nSize;

        // Write undo data
        long fileOutPos = ftell(fileout);
        if (fileOutPos < 0)
            return error("CBlockUndo::WriteToDisk() : ftell failed");
        pos.nPos = (unsigned int)fileOutPos;
        fileout << *this;

        // Flush stdio buffers and commit to disk before returning
        fflush(fileout);
        if (!IsInitialBlockDownload())
            FileCommit(fileout);

        return true;
    }

};

/** pruned version of CTransaction: only retains metadata and unspent transaction outputs
 *
 * Serialized format:
 * - VARINT(nVersion)
 * - VARINT(nFlags)
 * - VARINT(nCode)
 * - unspentness bitvector, for vout[2] and further; least significant byte first
 * - the non-spent CTxOuts (via CTxOutCompressor)
 * - VARINT(nHeight)
 * - VARINT(nTime + is_coinstake)
 * - VARINT(nBlockTime)
 *
 * The nFlag value consists of:
 * - bit 1: is coinbase
 * - bit 2: is coinstake
 * - bit 3: is pruned
 *
 * The nCode value consists of:
 * - bit 2: vout[0] is not spent
 * - bit 4: vout[1] is not spent
 * - The higher bits encode N, the number of non-zero bytes in the following bitvector.
 *   - In case both bit 2 and bit 4 are unset, they encode N-1, as there must be at
 *     least one non-spent output).
 *
 * Example: 010004835800816115944e077fe7c803cfa57f29b36bf87c1d358bb85e40f1d75240f1d752
 *          <><><><--------------------------------------------><----><------><------>
 *          |  | \                  |                            /      /       /
 *     version |  code            vout[1]                     height timestamp block timestamp
 *           flags
 *
 *    - version = 1
 *    - flags = 4
 *    - code = 4 (vout[1] is not spent, and 0 non-zero bytes of bitvector follow)
 *    - unspentness bitvector: as 0 non-zero bytes follow, it has length 0
 *    - vout[1]: 835800816115944e077fe7c803cfa57f29b36bf87c1d35
 *               * 8358: compact amount representation for 60000000000 (600 BTC)
 *               * 00: special txout type pay-to-pubkey-hash
 *               * 816115944e077fe7c803cfa57f29b36bf87c1d35: address uint160
 *    - height = 203998
 *    - time   = 1389883712
 *    - is_coinbase = 0
 *    - is_coinstake = 0
 *    - block time   = 1389883712
 *
 *
 * Example: 010508044086ef97d5790061b01caab50f1b8e9c50a5057eb43c2d9563a4eebbd123008c988f1a4a4de2161e0f50aac7f17e7f9555caa486af3b40f1d75240f1d752
 *          <><><><--><--------------------------------------------------><----------------------------------------------><----><------><------>
 *          /  | \   \                     |                                                           |                     /      /       /
 *     version | code unspentness       vout[4]                                                     vout[16]              height timestamp block timestamp
 *           flags
 *
 *  - version = 1
 *  - flags = 5
 *  - code = 8 (neither vout[0] or vout[1] are unspent,
 *                2 (1, +1 because both bit 2 and bit 4 are unset) non-zero bitvector bytes follow)
 *  - unspentness bitvector: bits 2 (0x04) and 14 (0x4000) are set, so vout[2+2] and vout[14+2] are unspent
 *  - vout[4]: 86ef97d5790061b01caab50f1b8e9c50a5057eb43c2d9563a4ee
 *             * 86ef97d579: compact amount representation for 234925952 (2.35 BTC)
 *             * 00: special txout type pay-to-pubkey-hash
 *             * 61b01caab50f1b8e9c50a5057eb43c2d9563a4ee: address uint160
 *  - vout[16]: bbd123008c988f1a4a4de2161e0f50aac7f17e7f9555caa4
 *              * bbd123: compact amount representation for 110397 (0.001 BTC)
 *              * 00: special txout type pay-to-pubkey-hash
 *              * 8c988f1a4a4de2161e0f50aac7f17e7f9555caa4: address uint160
 *  - height = 120891
 *  - time   = 1389883712
 *  - is_coinbase = 1
 *  - is_coinstake = 0
 *  - block time   = 1389883712
 *
 * Example: 010686af3b40f1d75240f1d752
 *          <><><----><------><------>
 *          /  |    \      |        \
 *   version flags height timestamp block timestamp
 *
 *  - version = 1
 *  - flags = 6 (00000110)
 *  - height = 120891
 *  - time   = 1389883712
 *  - is_coinbase = 0
 *  - is_coinstake = 1
 *  - block time   = 1389883712
 */
class CCoins
{
public:
    // whether transaction is a coinbase
    bool fCoinBase;

    // whether transaction is a coinstake
    bool fCoinStake;

    // unspent transaction outputs; spent outputs are .IsNull(); spent outputs at the end of the array are dropped
    std::vector<CTxOut> vout;

    // at which height this transaction was included in the active blockchain
    int nHeight;

    // version of the CTransaction; accesses to this value should probably check for nHeight as well,
    // as new tx version will probably only be introduced at certain heights
    int nVersion;

    // transaction timestamp + coinstake flag
    unsigned int nTime;

    // block timestamp
    unsigned int nBlockTime;

    // construct a CCoins from a CTransaction, at a given height/timestamp
    CCoins(const CTransaction &tx, int nHeightIn, int nBlockTimeIn) : fCoinBase(tx.IsCoinBase()), fCoinStake(tx.IsCoinStake()), vout(tx.vout), nHeight(nHeightIn), nVersion(tx.nVersion), nTime(tx.nTime), nBlockTime(nBlockTimeIn) { }

    // empty constructor
    CCoins() : fCoinBase(false), fCoinStake(false), vout(0), nHeight(0), nVersion(0), nTime(0), nBlockTime(0) { }

    // remove spent outputs at the end of vout
    void Cleanup() {
        while (vout.size() > 0 && (vout.back().IsNull() || vout.back().IsEmpty()))
            vout.pop_back();
    }

    // equality test
    friend bool operator==(const CCoins &a, const CCoins &b) {
         return a.fCoinBase == b.fCoinBase &&
                a.fCoinStake == b.fCoinStake &&
                a.nHeight == b.nHeight &&
                a.nVersion == b.nVersion &&
                a.nTime == b.nTime &&
                a.nBlockTime == b.nBlockTime &&
                a.vout == b.vout;
    }
    friend bool operator!=(const CCoins &a, const CCoins &b) {
        return !(a == b);
    }

    // calculate number of bytes for the bitmask, and its number of non-zero bytes
    // each bit in the bitmask represents the availability of one output, but the
    // availabilities of the first two outputs are encoded separately
    void CalcMaskSize(unsigned int &nBytes, unsigned int &nNonzeroBytes) const {
        unsigned int nLastUsedByte = 0;
        for (unsigned int b = 0; 2+b*8 < vout.size(); b++) {
            bool fZero = true;
            for (unsigned int i = 0; i < 8 && 2+b*8+i < vout.size(); i++) {
                if (!vout[2+b*8+i].IsNull()) {
                    fZero = false;
                    continue;
                }
            }
            if (!fZero) {
                nLastUsedByte = b + 1;
                nNonzeroBytes++;
            }
        }
        nBytes += nLastUsedByte;
    }

    bool IsCoinBase() const {
        return fCoinBase;
    }

    bool IsCoinStake() const {
        return fCoinStake;
    }

    unsigned int GetSerializeSize(int nType, int nVersion) const {
        unsigned int nSize = 0;
        bool fPruned = IsPruned();

        // version
        nSize += ::GetSerializeSize(VARINT(this->nVersion), nType, nVersion);
        unsigned char nFlags = 0;
        // coinbase, coinstake and prune flags
        nFlags = (fCoinBase ? 1 : 0)<<0 | (fCoinStake ? 1 : 0)<<1 | (fPruned ? 1 : 0)<<2;
        // size of flags
        nSize += ::GetSerializeSize(VARINT(nFlags), nType, nVersion);

        if (!IsPruned()) {
            unsigned int nMaskSize = 0, nMaskCode = 0;
            CalcMaskSize(nMaskSize, nMaskCode);
            bool fFirst = vout.size() > 0 && !vout[0].IsNull();
            bool fSecond = vout.size() > 1 && !vout[1].IsNull();

            assert(fFirst || fSecond || nMaskCode);
            unsigned int nCode = 8*(nMaskCode - (fFirst || fSecond ? 0 : 1)) + (fFirst ? 2 : 0) + (fSecond ? 4 : 0);
            // size of header code
            nSize += ::GetSerializeSize(VARINT(nCode), nType, nVersion);
            // spentness bitmask
            nSize += nMaskSize;
            // txouts themself
            for (unsigned int i = 0; i < vout.size(); i++)
                if (!vout[i].IsNull())
                    nSize += ::GetSerializeSize(CTxOutCompressor(REF(vout[i])), nType, nVersion);
            // height
            nSize += ::GetSerializeSize(VARINT(nHeight), nType, nVersion);
            // timestamp and coinstake flag
            nSize += ::GetSerializeSize(VARINT(nTime), nType, nVersion);
            // block timestamp
            nSize += ::GetSerializeSize(VARINT(nBlockTime), nType, nVersion);
        }
        else {
            // size of height
            nSize += ::GetSerializeSize(VARINT(nHeight), nType, nVersion);
            // size of timestamp
            nSize += ::GetSerializeSize(VARINT(nTime), nType, nVersion);
            // size of block timestamp
            nSize += ::GetSerializeSize(VARINT(nBlockTime), nType, nVersion);
        }

        return nSize;
    }

    template<typename Stream>
    void Serialize(Stream &s, int nType, int nVersion) const {
        bool fPruned = IsPruned();
        unsigned char nFlags = 0;
        nFlags = (fCoinBase ? 1 : 0)<<0 | (fCoinStake ? 1 : 0)<<1 | (fPruned ? 1 : 0)<<2;

        // version
        ::Serialize(s, VARINT(this->nVersion), nType, nVersion);
        // flags
        ::Serialize(s, VARINT(nFlags), nType, nVersion);

        if (!fPruned) {
            unsigned int nMaskSize = 0, nMaskCode = 0;
            CalcMaskSize(nMaskSize, nMaskCode);
            bool fFirst = vout.size() > 0 && !vout[0].IsNull();
            bool fSecond = vout.size() > 1 && !vout[1].IsNull();

            assert(fFirst || fSecond || nMaskCode);

            unsigned int nCode = 8*(nMaskCode - (fFirst || fSecond ? 0 : 1)) + (fFirst ? 2 : 0) + (fSecond ? 4 : 0);

            // header code
            ::Serialize(s, VARINT(nCode), nType, nVersion);
            // spentness bitmask
            for (unsigned int b = 0; b<nMaskSize; b++) {
                unsigned char chAvail = 0;
                for (unsigned int i = 0; i < 8 && 2+b*8+i < vout.size(); i++)
                    if (!vout[2+b*8+i].IsNull())
                        chAvail |= (1 << i);
                ::Serialize(s, chAvail, nType, nVersion);
            }
            // txouts themself
            for (unsigned int i = 0; i < vout.size(); i++) {
                if (!vout[i].IsNull())
                    ::Serialize(s, CTxOutCompressor(REF(vout[i])), nType, nVersion);
            }
            // coinbase height
            ::Serialize(s, VARINT(nHeight), nType, nVersion);
            // transaction timestamp and coinstake flag
            ::Serialize(s, VARINT(nTime), nType, nVersion);
            // block timestamp
            ::Serialize(s, VARINT(nBlockTime), nType, nVersion);
        }
        else {
            // coinbase height
            ::Serialize(s, VARINT(nHeight), nType, nVersion);
            // transaction timestamp
            ::Serialize(s, VARINT(nTime), nType, nVersion);
            // block timestamp
            ::Serialize(s, VARINT(nBlockTime), nType, nVersion);
        }
    }

    template<typename Stream>
    void Unserialize(Stream &s, int nType, int nVersion) {
        unsigned char nFlags = 0;

        // version
        ::Unserialize(s, VARINT(this->nVersion), nType, nVersion);
        // coinbase and coinstake flags
        ::Unserialize(s, VARINT(nFlags), nType, nVersion);

        fCoinBase = nFlags & (1<<0);
        fCoinStake = nFlags & (1<<1);
        bool fPruned = nFlags & (1<<2);

        if (!fPruned) {
            unsigned int nCode = 0;
            // header code
            ::Unserialize(s, VARINT(nCode), nType, nVersion);
            std::vector<bool> vAvail(2, false);
            vAvail[0] = nCode & 2;
            vAvail[1] = nCode & 4;
            unsigned int nMaskCode = (nCode / 8) + ((nCode & 6) != 0 ? 0 : 1);
            // spentness bitmask
            while (nMaskCode > 0) {
                unsigned char chAvail = 0;
                ::Unserialize(s, chAvail, nType, nVersion);
                for (unsigned int p = 0; p < 8; p++) {
                    bool f = (chAvail & (1 << p)) != 0;
                    vAvail.push_back(f);
                }
                if (chAvail != 0)
                    nMaskCode--;
            }
            // txouts themself
            vout.assign(vAvail.size(), CTxOut());
            for (unsigned int i = 0; i < vAvail.size(); i++) {
                if (vAvail[i])
                    ::Unserialize(s, REF(CTxOutCompressor(vout[i])), nType, nVersion);
            }
            // coinbase height
            ::Unserialize(s, VARINT(nHeight), nType, nVersion);
            // transaction timestamp
            ::Unserialize(s, VARINT(nTime), nType, nVersion);
            nTime = nTime;
            // block timestamp
            ::Unserialize(s, VARINT(nBlockTime), nType, nVersion);
        }
        else {
            // coinbase height
            ::Unserialize(s, VARINT(nHeight), nType, nVersion);
            // transaction timestamp
            ::Unserialize(s, VARINT(nTime), nType, nVersion);
            // block timestamp
            ::Unserialize(s, VARINT(nBlockTime), nType, nVersion);
        }
        Cleanup();
    }

    // mark an outpoint spent, and construct undo information
    bool Spend(const COutPoint &out, CTxInUndo &undo) {
        if (out.n >= vout.size())
            return false;
        if (vout[out.n].IsNull())
            return false;
        undo = CTxInUndo(vout[out.n]);
        vout[out.n].SetNull();
        Cleanup();
        if (vout.size() == 0) {
            undo.nHeight = nHeight;
            undo.nTime = nTime;
            undo.nBlockTime = nBlockTime;
            undo.fCoinBase = fCoinBase;
            undo.fCoinStake = fCoinStake;
            undo.nVersion = this->nVersion;
        }
        return true;
    }

    // mark a vout spent
    bool Spend(int nPos) {
        CTxInUndo undo;
        COutPoint out(0, nPos);
        return Spend(out, undo);
    }

    // check whether a particular output is still available
    bool IsAvailable(unsigned int nPos) const {
        return (nPos < vout.size() && !vout[nPos].IsNull());
    }

    // check whether the entire CCoins is spent
    // note that only !IsPruned() CCoins can be serialized
    bool IsPruned() const {
        if (vout.size() == 0)
            return true;

        BOOST_FOREACH(const CTxOut &out, vout)
            if (!out.IsNull())
                return false;

        return true;
    }
};




/** A transaction with a merkle branch linking it to the block chain. */
class CMerkleTx : public CTransaction
{
public:
    uint256 hashBlock;
    std::vector<uint256> vMerkleBranch;
    int nIndex;

    // memory only
    mutable bool fMerkleVerified;


    CMerkleTx()
    {
        Init();
    }

    CMerkleTx(const CTransaction& txIn) : CTransaction(txIn)
    {
        Init();
    }

    void Init()
    {
        hashBlock = 0;
        nIndex = -1;
        fMerkleVerified = false;
    }


    IMPLEMENT_SERIALIZE
    (
        nSerSize += SerReadWrite(s, *(CTransaction*)this, nType, nVersion, ser_action);
        nVersion = this->nVersion;
        READWRITE(hashBlock);
        READWRITE(vMerkleBranch);
        READWRITE(nIndex);
    )


    int SetMerkleBranch(const CBlock* pblock=NULL);
    int GetDepthInMainChain(CBlockIndex* &pindexRet) const;
    int GetDepthInMainChain() const { CBlockIndex *pindexRet; return GetDepthInMainChain(pindexRet); }
    bool IsInMainChain() const { return GetDepthInMainChain() > 0; }
    int GetBlocksToMaturity() const;
    bool AcceptToMemoryPool(bool fCheckInputs=true);
};



/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlock
{
public:
    // header
    static const int CURRENT_VERSION=6;
    int nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    unsigned int nTime;
    unsigned int nBits;
    unsigned int nNonce;

    // network and disk
    std::vector<CTransaction> vtx;

    // ppcoin: block signature - signed by one of the coin base txout[N]'s owner
    std::vector<unsigned char> vchBlockSig;

    // memory only
    mutable std::vector<uint256> vMerkleTree;

    // Denial-of-service detection:
    mutable int nDoS;
    bool DoS(int nDoSIn, bool fIn) const { nDoS += nDoSIn; return fIn; }

    CBlock()
    {
        SetNull();
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);

        // ConnectBlock depends on vtx following header to generate CDiskTxPos
        if (!(nType & (SER_GETHASH|SER_BLOCKHEADERONLY)))
        {
            READWRITE(vtx);
            READWRITE(vchBlockSig);
        }
        else if (fRead)
        {
            const_cast<CBlock*>(this)->vtx.clear();
            const_cast<CBlock*>(this)->vchBlockSig.clear();
        }
    )

    void SetNull()
    {
        nVersion = CBlock::CURRENT_VERSION;
        hashPrevBlock = 0;
        hashMerkleRoot = 0;
        nTime = 0;
        nBits = 0;
        nNonce = 0;
        vtx.clear();
        vchBlockSig.clear();
        vMerkleTree.clear();
        nDoS = 0;
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const
    {
        return scrypt_blockhash(CVOIDBEGIN(nVersion));
    }

    int64 GetBlockTime() const
    {
        return (int64)nTime;
    }

    void UpdateTime(const CBlockIndex* pindexPrev);

    unsigned int GetStakeEntropyBit(unsigned int nTime) const;

    // two types of block: proof-of-work or proof-of-stake
    bool IsProofOfStake() const
    {
        return (vtx.size() > 1 && vtx[1].IsCoinStake());
    }

    bool IsProofOfWork() const
    {
        return !IsProofOfStake();
    }

    std::pair<COutPoint, unsigned int> GetProofOfStake() const
    {
        return IsProofOfStake()? std::make_pair(vtx[1].vin[0].prevout, vtx[1].nTime) : std::make_pair(COutPoint(), (unsigned int)0);
    }

    // get max transaction timestamp
    int64 GetMaxTransactionTime() const
    {
        int64 maxTransactionTime = 0;
        BOOST_FOREACH(const CTransaction& tx, vtx)
            maxTransactionTime = std::max(maxTransactionTime, (int64)tx.nTime);
        return maxTransactionTime;
    }

    uint256 BuildMerkleTree() const
    {
        vMerkleTree.clear();
        BOOST_FOREACH(const CTransaction& tx, vtx)
            vMerkleTree.push_back(tx.GetHash());
        int j = 0;
        for (int nSize = vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
        {
            for (int i = 0; i < nSize; i += 2)
            {
                int i2 = std::min(i+1, nSize-1);
                vMerkleTree.push_back(Hash(BEGIN(vMerkleTree[j+i]),  END(vMerkleTree[j+i]),
                                           BEGIN(vMerkleTree[j+i2]), END(vMerkleTree[j+i2])));
            }
            j += nSize;
        }
        return (vMerkleTree.empty() ? 0 : vMerkleTree.back());
    }

    const uint256 &GetTxHash(unsigned int nIndex) const {
        assert(vMerkleTree.size() > 0); // BuildMerkleTree must have been called first
        assert(nIndex < vtx.size());
        return vMerkleTree[nIndex];
    }

    std::vector<uint256> GetMerkleBranch(int nIndex) const
    {
        if (vMerkleTree.empty())
            BuildMerkleTree();
        std::vector<uint256> vMerkleBranch;
        int j = 0;
        for (int nSize = vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
        {
            int i = std::min(nIndex^1, nSize-1);
            vMerkleBranch.push_back(vMerkleTree[j+i]);
            nIndex >>= 1;
            j += nSize;
        }
        return vMerkleBranch;
    }

    static uint256 CheckMerkleBranch(uint256 hash, const std::vector<uint256>& vMerkleBranch, int nIndex)
    {
        if (nIndex == -1)
            return 0;
        BOOST_FOREACH(const uint256& otherside, vMerkleBranch)
        {
            if (nIndex & 1)
                hash = Hash(BEGIN(otherside), END(otherside), BEGIN(hash), END(hash));
            else
                hash = Hash(BEGIN(hash), END(hash), BEGIN(otherside), END(otherside));
            nIndex >>= 1;
        }
        return hash;
    }

    bool WriteToDisk(CDiskBlockPos &pos)
    {
        // Open history file to append
        CAutoFile fileout = CAutoFile(OpenBlockFile(pos), SER_DISK, CLIENT_VERSION);
        if (!fileout)
            return error("CBlock::WriteToDisk() : OpenBlockFile failed");

        // Write index header
        unsigned int nSize = fileout.GetSerializeSize(*this);
        fileout << FLATDATA(pchMessageStart) << nSize;

        // Write block
        long fileOutPos = ftell(fileout);
        if (fileOutPos < 0)
            return error("CBlock::WriteToDisk() : ftell failed");
        pos.nPos = (unsigned int)fileOutPos;
        fileout << *this;

        // Flush stdio buffers and commit to disk before returning
        fflush(fileout);
        if (!IsInitialBlockDownload())
            FileCommit(fileout);

        return true;
    }

    bool ReadFromDisk(const CDiskBlockPos &pos, bool fReadTransactions = true)
    {
        SetNull();

        // Open history file to read
        CAutoFile filein = CAutoFile(OpenBlockFile(pos, true), SER_DISK, CLIENT_VERSION);
        if (!filein)
            return error("CBlock::ReadFromDisk() : OpenBlockFile failed");
        if (!fReadTransactions)
            filein.nType |= SER_BLOCKHEADERONLY;

        // Read block
        try {
            filein >> *this;
        }
        catch (std::exception &e) {
            return error("%s() : deserialize or I/O error", __PRETTY_FUNCTION__);
        }

        // Check the header
        if (fReadTransactions && IsProofOfWork() && !CheckProofOfWork(GetHash(), nBits))
            return error("CBlock::ReadFromDisk() : errors in block header");

        return true;
    }

    void print() const
    {
        printf("CBlock(hash=%s, ver=%d, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%"PRIszu", vchBlockSig=%s)\n",
            GetHash().ToString().c_str(),
            nVersion,
            hashPrevBlock.ToString().c_str(),
            hashMerkleRoot.ToString().c_str(),
            nTime, nBits, nNonce,
            vtx.size(),
            HexStr(vchBlockSig.begin(), vchBlockSig.end()).c_str());
        for (unsigned int i = 0; i < vtx.size(); i++)
        {
            printf("  ");
            vtx[i].print();
        }
        printf("  vMerkleTree: ");
        for (unsigned int i = 0; i < vMerkleTree.size(); i++)
            printf("%s ", vMerkleTree[i].ToString().substr(0,10).c_str());
        printf("\n");
    }

    // Undo the effects of this block (with given index) on the UTXO set represented by coins
    bool DisconnectBlock(CBlockIndex *pindex, CCoinsViewCache &coins);

    // Apply the effects of this block (with given index) on the UTXO set represented by coins
    bool ConnectBlock(CBlockIndex *pindex, CCoinsViewCache &coins, bool fJustCheck=false);

    // Read a block from disk
    bool ReadFromDisk(const CBlockIndex* pindex, bool fReadTransactions=true);

    // Add this block to the block index, and if necessary, switch the active block chain to this
    bool AddToBlockIndex(const CDiskBlockPos &pos);

    // Context-independent validity checks
    bool CheckBlockHeader(bool fCheckPoW=true, bool fCheckSig=false) const;
    bool CheckBlock(bool fCheckPOW=true, bool fCheckMerkleRoot=true, bool fCheckSig=false) const;

    // Store block on disk
    bool AcceptBlock();

    // Get total coinage consumed
    bool GetCoinAge(uint64& nCoinAge) const;

    // Generate proof-of-stake block signature
    bool SignBlock(CWallet& keystore);

    // Get generator key
    bool GetGenerator(CKey& GeneratorKey) const;

    // Validate proof-of-stake block signature
    bool CheckSignature(bool& fFatal, uint256& hashProofOfStake) const;

    // Legacy proof-of-work signature
    bool CheckLegacySignature() const;
};


class CBlockFileInfo
{
public:
    unsigned int nBlocks;      // number of blocks stored in file
    unsigned int nSize;        // number of used bytes of block file
    unsigned int nUndoSize;    // number of used bytes in the undo file
    unsigned int nHeightFirst; // lowest height of block in file
    unsigned int nHeightLast;  // highest height of block in file
    uint64 nTimeFirst;         // earliest time of block in file
    uint64 nTimeLast;          // latest time of block in file

    IMPLEMENT_SERIALIZE(
        READWRITE(VARINT(nBlocks));
        READWRITE(VARINT(nSize));
        READWRITE(VARINT(nUndoSize));
        READWRITE(VARINT(nHeightFirst));
        READWRITE(VARINT(nHeightLast));
        READWRITE(VARINT(nTimeFirst));
        READWRITE(VARINT(nTimeLast));
     )

     void SetNull() {
         nBlocks = 0;
         nSize = 0;
         nUndoSize = 0;
         nHeightFirst = 0;
         nHeightLast = 0;
         nTimeFirst = 0;
         nTimeLast = 0;
     }

     CBlockFileInfo() {
         SetNull();
     }

     std::string ToString() const {
         return strprintf("CBlockFileInfo(blocks=%u, size=%u, heights=%u..%u, time=%s..%s)", nBlocks, nSize, nHeightFirst, nHeightLast, DateTimeStrFormat("%Y-%m-%d", nTimeFirst).c_str(), DateTimeStrFormat("%Y-%m-%d", nTimeLast).c_str());
     }

     // update statistics (does not update nSize)
     void AddBlock(unsigned int nHeightIn, uint64 nTimeIn) {
         if (nBlocks==0 || nHeightFirst > nHeightIn)
             nHeightFirst = nHeightIn;
         if (nBlocks==0 || nTimeFirst > nTimeIn)
             nTimeFirst = nTimeIn;
         nBlocks++;
         if (nHeightIn > nHeightFirst)
             nHeightLast = nHeightIn;
         if (nTimeIn > nTimeLast)
             nTimeLast = nTimeIn;
     }
};


extern CCriticalSection cs_LastBlockFile;
extern CBlockFileInfo infoLastBlockFile;
extern int nLastBlockFile;

enum BlockStatus {
    BLOCK_VALID_UNKNOWN      =    0,
    BLOCK_VALID_HEADER       =    1, // parsed, version ok, hash satisfies claimed PoW, 1 <= vtx count <= max, timestamp not in future
    BLOCK_VALID_TREE         =    2, // parent found, difficulty matches, timestamp >= median previous, checkpoint
    BLOCK_VALID_TRANSACTIONS =    3, // only first tx is coinbase, 2 <= coinbase input script length <= 100, transactions valid, no duplicate txids, sigops, size, merkle root
    BLOCK_VALID_CHAIN        =    4, // outputs do not overspend inputs, no double spends, coinbase output ok, immature coinbase spends, BIP30
    BLOCK_VALID_SCRIPTS      =    5, // scripts/signatures ok
    BLOCK_VALID_MASK         =    7,

    BLOCK_HAVE_DATA          =    8, // full block available in blk*.dat
    BLOCK_HAVE_UNDO          =   16, // undo data available in rev*.dat
    BLOCK_HAVE_MASK          =   24,

    BLOCK_FAILED_VALID       =   32, // stage after last reached validness failed
    BLOCK_FAILED_CHILD       =   64, // descends from failed block
    BLOCK_FAILED_MASK        =   96
};

/** The block chain is a tree shaped structure starting with the
 * genesis block at the root, with each block potentially having multiple
 * candidates to be the next block.  pprev and pnext link a path through the
 * main/longest chain.  A blockindex may have multiple pprev pointing back
 * to it, but pnext will only point forward to the longest branch, or will
 * be null if the block is not part of the longest chain.
 */
class CBlockIndex
{
public:
    // pointer to the hash of the block, if any. memory is owned by this CBlockIndex
    const uint256* phashBlock;

    // pointer to the index of the predecessor of this block
    CBlockIndex* pprev;

    // (memory only) pointer to the index of the *active* successor of this block
    CBlockIndex* pnext;

    // height of the entry in the chain. The genesis block has height 0
    int nHeight;

    // Which # file this block is stored in (blk?????.dat)
    int nFile;

    // Byte offset within blk?????.dat where this block's data is stored
    unsigned int nDataPos;

    // Byte offset within rev?????.dat where this block's undo data is stored
    unsigned int nUndoPos;

    // (memory only) Trust score of block chain up to and including this block
    uint256 nChainTrust;

    // Number of transactions in this block.
    unsigned int nTx;

    // (memory only) Number of transactions in the chain up to and including this block
    unsigned int nChainTx;

    // Verification status of this block. See enum BlockStatus for detailed info
    unsigned int nStatus;

    // Coins amount created by this block
    int64 nMint;

    // Total coins created in this block chain up to and including this block
    int64 nMoneySupply;

    // Block flags
    unsigned int nFlags;
    enum
    {
        // is proof-of-stake block
        BLOCK_PROOF_OF_STAKE = (1 << 0),
        // entropy bit for stake modifier
        BLOCK_STAKE_ENTROPY  = (1 << 1),
        // regenerated stake modifier
        BLOCK_STAKE_MODIFIER = (1 << 2),
    };

    // Hash modifier for proof-of-stake kernel
    uint64 nStakeModifier;

    // Checksum of index in-memory only
    unsigned int nStakeModifierChecksum;

    // Predecessor of coinstake transaction
    COutPoint prevoutStake;

    // Timestamp of coinstake transaction
    unsigned int nStakeTime;

    // Kernel hash
    uint256 hashProofOfStake;

    // Block header
    int nVersion;
    uint256 hashMerkleRoot;
    unsigned int nTime;
    unsigned int nBits;
    unsigned int nNonce;

    CBlockIndex()
    {
        phashBlock = NULL;
        pprev = NULL;
        pnext = NULL;
        nHeight = 0;
        nFile = 0;
        nDataPos = 0;
        nUndoPos = 0;
        nChainTrust = 0;
        nTx = 0;
        nChainTx = 0;
        nStatus = 0;
        nMint = 0;
        nMoneySupply = 0;
        nFlags = 0;
        nStakeModifier = 0;
        nStakeModifierChecksum = 0;
        hashProofOfStake = 0;
        prevoutStake.SetNull();
        nStakeTime = 0;

        nVersion       = 0;
        hashMerkleRoot = 0;
        nTime          = 0;
        nBits          = 0;
        nNonce         = 0;
    }

    CBlockIndex(CBlock& block)
    {
        phashBlock = NULL;
        pprev = NULL;
        pnext = NULL;
        nHeight = 0;
        nFile = 0;
        nDataPos = 0;
        nUndoPos = 0;
        nChainTrust = 0;
        nTx = 0;
        nChainTx = 0;
        nStatus = 0;
        nMint = 0;
        nMoneySupply = 0;
        nFlags = 0;
        nStakeModifier = 0;
        nStakeModifierChecksum = 0;
        hashProofOfStake = 0;
        if (block.IsProofOfStake())
        {
            SetProofOfStake();
            prevoutStake = block.vtx[1].vin[0].prevout;
            nStakeTime = block.vtx[1].nTime;
        }
        else
        {
            prevoutStake.SetNull();
            nStakeTime = 0;
        }

        nVersion       = block.nVersion;
        hashMerkleRoot = block.hashMerkleRoot;
        nTime          = block.nTime;
        nBits          = block.nBits;
        nNonce         = block.nNonce;
    }

    CDiskBlockPos GetBlockPos() const {
        CDiskBlockPos ret;
        if (nStatus & BLOCK_HAVE_DATA) {
            ret.nFile = nFile;
            ret.nPos  = nDataPos;
        } else
            ret.SetNull();
        return ret;
    }

    CDiskBlockPos GetUndoPos() const {
        CDiskBlockPos ret;
        if (nStatus & BLOCK_HAVE_UNDO) {
            ret.nFile = nFile;
            ret.nPos  = nUndoPos;
        } else
            ret.SetNull();
        return ret;
    }

    CBlock GetBlockHeader() const
    {
        CBlock block;
        block.nVersion       = nVersion;
        if (pprev)
            block.hashPrevBlock = pprev->GetBlockHash();
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;
        block.nNonce         = nNonce;
        return block;
    }

    uint256 GetBlockHash() const
    {
        return *phashBlock;
    }

    int64 GetBlockTime() const
    {
        return (int64)nTime;
    }

    uint256 GetBlockTrust() const;

    bool IsInMainChain() const
    {
        return (pnext || this == pindexBest);
    }

    bool CheckIndex() const
    {
        return true;
    }

    enum { nMedianTimeSpan=11 };

    int64 GetMedianTimePast() const
    {
        int64 pmedian[nMedianTimeSpan];
        int64* pbegin = &pmedian[nMedianTimeSpan];
        int64* pend = &pmedian[nMedianTimeSpan];

        const CBlockIndex* pindex = this;
        for (int i = 0; i < nMedianTimeSpan && pindex; i++, pindex = pindex->pprev)
            *(--pbegin) = pindex->GetBlockTime();

        std::sort(pbegin, pend);
        return pbegin[(pend - pbegin)/2];
    }

    int64 GetMedianTime() const
    {
        const CBlockIndex* pindex = this;
        for (int i = 0; i < nMedianTimeSpan/2; i++)
        {
            if (!pindex->pnext)
                return GetBlockTime();
            pindex = pindex->pnext;
        }
        return pindex->GetMedianTimePast();
    }

    /**
     * Returns true if there are nRequired or more blocks of minVersion or above
     * in the last nToCheck blocks, starting at pstart and going backwards.
     */
    static bool IsSuperMajority(int minVersion, const CBlockIndex* pstart,
                                unsigned int nRequired, unsigned int nToCheck);

    bool IsProofOfWork() const
    {
        return !(nFlags & BLOCK_PROOF_OF_STAKE);
    }

    bool IsProofOfStake() const
    {
        return (nFlags & BLOCK_PROOF_OF_STAKE);
    }

    void SetProofOfStake()
    {
        nFlags |= BLOCK_PROOF_OF_STAKE;
    }

    unsigned int GetStakeEntropyBit() const
    {
        return ((nFlags & BLOCK_STAKE_ENTROPY) >> 1);
    }

    bool SetStakeEntropyBit(unsigned int nEntropyBit)
    {
        if (nEntropyBit > 1)
            return false;
        nFlags |= (nEntropyBit? BLOCK_STAKE_ENTROPY : 0);
        return true;
    }

    bool GeneratedStakeModifier() const
    {
        return (nFlags & BLOCK_STAKE_MODIFIER);
    }

    void SetStakeModifier(uint64 nModifier, bool fGeneratedStakeModifier)
    {
        nStakeModifier = nModifier;
        if (fGeneratedStakeModifier)
            nFlags |= BLOCK_STAKE_MODIFIER;
    }

    std::string ToString() const
    {
        return strprintf("CBlockIndex(nprev=%p, pnext=%p nHeight=%d, nMint=%s, nMoneySupply=%s, nFlags=(%s)(%d)(%s), nStakeModifier=%016"PRI64x", nStakeModifierChecksum=%08x, hashProofOfStake=%s, prevoutStake=(%s), nStakeTime=%d merkle=%s, hashBlock=%s)",
            pprev, pnext, nHeight,
            FormatMoney(nMint).c_str(), FormatMoney(nMoneySupply).c_str(),
            GeneratedStakeModifier() ? "MOD" : "-", GetStakeEntropyBit(), IsProofOfStake()? "PoS" : "PoW",
            nStakeModifier, nStakeModifierChecksum, 
            hashProofOfStake.ToString().c_str(),
            prevoutStake.ToString().c_str(), nStakeTime,
            hashMerkleRoot.ToString().c_str(),
            GetBlockHash().ToString().c_str());
    }


    void print() const
    {
        printf("%s\n", ToString().c_str());
    }
};

struct CBlockIndexTrustComparator
{
    bool operator()(CBlockIndex *pa, CBlockIndex *pb) {
        if (pa->nChainTrust > pb->nChainTrust) return false;
        if (pa->nChainTrust < pb->nChainTrust) return true;

        return false; // identical blocks
    }
};

/** Used to marshal pointers into hashes for db storage. */
class CDiskBlockIndex : public CBlockIndex
{
private:
    uint256 blockHash;
public:
    uint256 hashPrev;

    CDiskBlockIndex() {
        hashPrev = 0;
        blockHash = 0;
    }

    explicit CDiskBlockIndex(CBlockIndex* pindex) : CBlockIndex(*pindex) {
        hashPrev = (pprev ? pprev->GetBlockHash() : 0);
    }

    IMPLEMENT_SERIALIZE
    (
        if (!(nType & SER_GETHASH))
            READWRITE(VARINT(nVersion));

        READWRITE(VARINT(nHeight));
        READWRITE(VARINT(nStatus));
        READWRITE(VARINT(nTx));
        if (nStatus & (BLOCK_HAVE_DATA | BLOCK_HAVE_UNDO))
            READWRITE(VARINT(nFile));
        if (nStatus & BLOCK_HAVE_DATA)
            READWRITE(VARINT(nDataPos));
        if (nStatus & BLOCK_HAVE_UNDO)
            READWRITE(VARINT(nUndoPos));
        READWRITE(nMint);
        READWRITE(nMoneySupply);
        READWRITE(nFlags);
        READWRITE(nStakeModifier);
        if (IsProofOfStake())
        {
            READWRITE(prevoutStake);
            READWRITE(nStakeTime);
            READWRITE(hashProofOfStake);
        }
        else if (fRead)
        {
            const_cast<CDiskBlockIndex*>(this)->prevoutStake.SetNull();
            const_cast<CDiskBlockIndex*>(this)->nStakeTime = 0;
            const_cast<CDiskBlockIndex*>(this)->hashProofOfStake = 0;
        }
        READWRITE(blockHash);

        // block header
        READWRITE(this->nVersion);
        READWRITE(hashPrev);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
    )

    uint256 GetBlockHash() const
    {
        if (fUseFastIndex && (nTime < GetAdjustedTime() - 24 * 60 * 60) && blockHash != 0)
            return blockHash;

        CBlock block;
        block.nVersion        = nVersion;
        block.hashPrevBlock   = hashPrev;
        block.hashMerkleRoot  = hashMerkleRoot;
        block.nTime           = nTime;
        block.nBits           = nBits;
        block.nNonce          = nNonce;

        const_cast<CDiskBlockIndex*>(this)->blockHash = block.GetHash();

        return blockHash;
    }


    std::string ToString() const
    {
        std::string str = "CDiskBlockIndex(";
        str += CBlockIndex::ToString();
        str += strprintf("\n                hashBlock=%s, hashPrev=%s)",
            GetBlockHash().ToString().c_str(),
            hashPrev.ToString().substr(0,20).c_str());
        return str;
    }

    void print() const
    {
        printf("%s\n", ToString().c_str());
    }
};


/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
class CBlockLocator
{
protected:
    std::vector<uint256> vHave;
public:

    CBlockLocator()
    {
    }

    explicit CBlockLocator(const CBlockIndex* pindex)
    {
        Set(pindex);
    }

    explicit CBlockLocator(uint256 hashBlock)
    {
        std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
        if (mi != mapBlockIndex.end())
            Set((*mi).second);
    }

    CBlockLocator(const std::vector<uint256>& vHaveIn)
    {
        vHave = vHaveIn;
    }

    IMPLEMENT_SERIALIZE
    (
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    )

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull()
    {
        return vHave.empty();
    }

    void Set(const CBlockIndex* pindex)
    {
        vHave.clear();
        int nStep = 1;
        while (pindex)
        {
            vHave.push_back(pindex->GetBlockHash());

            // Exponentially larger steps back
            for (int i = 0; pindex && i < nStep; i++)
                pindex = pindex->pprev;
            if (vHave.size() > 10)
                nStep *= 2;
        }
        vHave.push_back((!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet));
    }

    int GetDistanceBack()
    {
        // Retrace how far back it was in the sender's branch
        int nDistance = 0;
        int nStep = 1;
        BOOST_FOREACH(const uint256& hash, vHave)
        {
            std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
            if (mi != mapBlockIndex.end())
            {
                CBlockIndex* pindex = (*mi).second;
                if (pindex->IsInMainChain())
                    return nDistance;
            }
            nDistance += nStep;
            if (nDistance > 10)
                nStep *= 2;
        }
        return nDistance;
    }

    CBlockIndex* GetBlockIndex()
    {
        // Find the first block the caller has in the main chain
        BOOST_FOREACH(const uint256& hash, vHave)
        {
            std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
            if (mi != mapBlockIndex.end())
            {
                CBlockIndex* pindex = (*mi).second;
                if (pindex->IsInMainChain())
                    return pindex;
            }
        }
        return pindexGenesisBlock;
    }

    uint256 GetBlockHash()
    {
        // Find the first block the caller has in the main chain
        BOOST_FOREACH(const uint256& hash, vHave)
        {
            std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
            if (mi != mapBlockIndex.end())
            {
                CBlockIndex* pindex = (*mi).second;
                if (pindex->IsInMainChain())
                    return hash;
            }
        }
        return (!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet);
    }

    int GetHeight()
    {
        CBlockIndex* pindex = GetBlockIndex();
        if (!pindex)
            return 0;
        return pindex->nHeight;
    }
};





class CTxMemPool
{
public:
    mutable CCriticalSection cs;
    std::map<uint256, CTransaction> mapTx;
    std::map<COutPoint, CInPoint> mapNextTx;

    bool accept(CTransaction &tx, bool fCheckInputs, bool* pfMissingInputs);
    bool addUnchecked(const uint256& hash, CTransaction &tx);
    bool remove(CTransaction &tx);
    void clear();
    void queryHashes(std::vector<uint256>& vtxid);
    void pruneSpent(const uint256& hash, CCoins &coins);

    unsigned long size()
    {
        LOCK(cs);
        return mapTx.size();
    }

    bool exists(uint256 hash)
    {
        return (mapTx.count(hash) != 0);
    }

    CTransaction& lookup(uint256 hash)
    {
        return mapTx[hash];
    }
};

extern CTxMemPool mempool;

struct CCoinsStats
{
    int nHeight;
    uint64 nTransactions;
    uint64 nPrunedTransactions;
    uint64 nTransactionOutputs;
    uint64 nSerializedSize;

    CCoinsStats() : nHeight(0), nTransactions(0), nPrunedTransactions(0), nTransactionOutputs(0), nSerializedSize(0) {}
};

/** Abstract view on the open txout dataset. */
class CCoinsView
{
public:
    // Retrieve the CCoins (unspent transaction outputs) for a given txid
    virtual bool GetCoins(uint256 txid, CCoins &coins);

    // Modify the CCoins for a given txid
    virtual bool SetCoins(uint256 txid, const CCoins &coins);

    // Just check whether we have data for a given txid.
    // This may (but cannot always) return true for fully spent transactions
    virtual bool HaveCoins(uint256 txid);

    // Retrieve the block index whose state this CCoinsView currently represents
    virtual CBlockIndex *GetBestBlock();

    // Modify the currently active block index
    virtual bool SetBestBlock(CBlockIndex *pindex);
    virtual bool BatchWrite(const std::map<uint256, CCoins> &mapCoins, CBlockIndex *pindex);
    virtual bool GetStats(CCoinsStats &stats);
};

/** CCoinsView backed by another CCoinsView */
class CCoinsViewBacked : public CCoinsView
{
protected:
    CCoinsView *base;

public:
    CCoinsViewBacked(CCoinsView &viewIn);
    bool GetCoins(uint256 txid, CCoins &coins);
    bool SetCoins(uint256 txid, const CCoins &coins);
    bool HaveCoins(uint256 txid);
    CBlockIndex *GetBestBlock();
    bool SetBestBlock(CBlockIndex *pindex);
    void SetBackend(CCoinsView &viewIn);
    bool BatchWrite(const std::map<uint256, CCoins> &mapCoins, CBlockIndex *pindex);
    bool GetStats(CCoinsStats &stats);
};

/** CCoinsView that adds a memory cache for transactions to another CCoinsView */
class CCoinsViewCache : public CCoinsViewBacked
{
protected:
    CBlockIndex *pindexTip;
    std::map<uint256,CCoins> cacheCoins;
    std::map<uint256,CCoins> cacheCoinsReadOnly;

public:
    CCoinsViewCache(CCoinsView &baseIn, bool fDummy = false);
    bool GetCoins(uint256 txid, CCoins &coins);
    bool GetCoinsReadOnly(uint256 txid, CCoins &coins);
    bool SetCoins(uint256 txid, const CCoins &coins);
    bool HaveCoins(uint256 txid);
    CCoins &GetCoins(uint256 txid);
    CBlockIndex *GetBestBlock();
    bool SetBestBlock(CBlockIndex *pindex);
    bool BatchWrite(const std::map<uint256, CCoins> &mapCoins, CBlockIndex *pindex);
    bool Flush();
    unsigned int GetCacheSize();
private:
    std::map<uint256,CCoins>::iterator FetchCoins(uint256 txid);
};

/** CCoinsView that brings transactions from a memorypool into view.
    It does not check for spendings by memory pool transactions. */
class CCoinsViewMemPool : public CCoinsViewBacked
{
protected:
    CTxMemPool &mempool;

public:
    CCoinsViewMemPool(CCoinsView &baseIn, CTxMemPool &mempoolIn);
    bool GetCoins(uint256 txid, CCoins &coins);
    bool HaveCoins(uint256 txid);
};

/** Global variable that points to the active CCoinsView (protected by cs_main) */
extern CCoinsViewCache *pcoinsTip;

/** Global variable that points to the active block tree (protected by cs_main) */
extern CBlockTreeDB *pblocktree;

#endif
