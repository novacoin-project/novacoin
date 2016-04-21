// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_CHECKPOINT_H
#define  BITCOIN_CHECKPOINT_H

#include <map>
#include "util.h"
#include "net.h"

// max 1 hour before latest block
static const int64_t CHECKPOINT_MAX_SPAN = nOneHour;

class uint256;
class CBlockIndex;
class CSyncCheckpoint;

// Block-chain checkpoints are compiled-in sanity checks.
// They are updated every release or three.
namespace Checkpoints
{
    // Checkpointing mode
    enum CPMode
    {
        // Scrict checkpoints policy, perform conflicts verification and resolve conflicts
        CP_STRICT = 0,
        // Advisory checkpoints policy, perform conflicts verification but don't try to resolve them
        CP_ADVISORY = 1,
        // Permissive checkpoints policy, don't perform any checking
        CP_PERMISSIVE = 2
    };

    // Returns true if block passes checkpoint checks
    bool CheckHardened(int nHeight, const uint256& hash);

    // Returns true if block passes banlist checks
    bool CheckBanned(const uint256 &nHash);

    // Return conservative estimate of total number of blocks, 0 if unknown
    int GetTotalBlocksEstimate();

    // Returns last CBlockIndex* in mapBlockIndex that is a checkpoint
    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex);

    // Returns last checkpoint timestamp
    unsigned int GetLastCheckpointTime();

    extern uint256 hashSyncCheckpoint;
    extern CSyncCheckpoint checkpointMessage;
    extern uint256 hashInvalidCheckpoint;
    extern CCriticalSection cs_hashSyncCheckpoint;

    CBlockIndex* GetLastSyncCheckpoint();
    bool WriteSyncCheckpoint(const uint256& hashCheckpoint);
    bool AcceptPendingSyncCheckpoint();
    uint256 AutoSelectSyncCheckpoint();
    bool CheckSync(const uint256& hashBlock, const CBlockIndex* pindexPrev);
    bool WantedByPendingSyncCheckpoint(uint256 hashBlock);
    bool ResetSyncCheckpoint();
    void AskForPendingSyncCheckpoint(CNode* pfrom);
    bool SetCheckpointPrivKey(std::string strPrivKey);
    bool SendSyncCheckpoint(uint256 hashCheckpoint);
    bool IsMatureSyncCheckpoint();
}

// ppcoin: synchronized checkpoint
class CUnsignedSyncCheckpoint
{
public:
    int32_t nVersion;
    uint256 hashCheckpoint;      // checkpoint block

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(hashCheckpoint);
    )

    void SetNull();
    std::string ToString() const;
};

class CSyncCheckpoint : public CUnsignedSyncCheckpoint
{
public:
    static const std::string strMasterPubKey;
    static std::string strMasterPrivKey;

    std::vector<unsigned char> vchMsg;
    std::vector<unsigned char> vchSig;

    CSyncCheckpoint();

    IMPLEMENT_SERIALIZE
    (
        READWRITE(vchMsg);
        READWRITE(vchSig);
    )

    void SetNull();
    bool IsNull() const;
    uint256 GetHash() const;
    bool RelayTo(CNode* pnode) const;
    bool CheckSignature();
    bool ProcessSyncCheckpoint(CNode* pfrom);
};

#endif
