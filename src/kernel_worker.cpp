#include <vector>
#include <inttypes.h>

#include "uint256.h"
#include "bignum.h"
#include "kernel.h"
#include "kernel_worker.h"

using namespace std;

KernelWorker::KernelWorker(unsigned char *kernel, uint32_t nBits, uint32_t nInputTxTime, int64_t nValueIn, uint32_t nIntervalBegin, uint32_t nIntervalEnd) 
        : kernel(kernel), nBits(nBits), nInputTxTime(nInputTxTime), bnValueIn(nValueIn), nIntervalBegin(nIntervalBegin), nIntervalEnd(nIntervalEnd)
    {
        solutions = vector<pair<uint256,uint32_t> >();
    }

void KernelWorker::Do_generic()
{
    SetThreadPriority(THREAD_PRIORITY_LOWEST);

    // Compute maximum possible target to filter out majority of obviously insufficient hashes
    CBigNum bnTargetPerCoinDay;
    bnTargetPerCoinDay.SetCompact(nBits);
    auto nMaxTarget = (bnTargetPerCoinDay * bnValueIn * nStakeMaxAge / COIN / nOneDay).getuint256();

    SHA256_CTX ctx, workerCtx;
    // Init new sha256 context and update it
    //   with first 24 bytes of kernel
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, kernel, 8 + 16);
    workerCtx = ctx; // save context

    // Sha256 result buffer
    uint32_t hashProofOfStake[8];
    auto pnHashProofOfStake = (uint256 *)&hashProofOfStake;

    // Search forward in time from the given timestamp
    // Stopping search in case of shutting down
    for (auto nTimeTx=nIntervalBegin, nMaxTarget32 = nMaxTarget.Get32(7); nTimeTx<nIntervalEnd && !fShutdown; nTimeTx++)
    {
        // Complete first hashing iteration
        uint256 hash1;
        SHA256_Update(&ctx, (unsigned char*)&nTimeTx, 4);
        SHA256_Final((unsigned char*)&hash1, &ctx);

        // Restore context
        ctx = workerCtx;

        // Finally, calculate kernel hash
        SHA256((unsigned char*)&hash1, sizeof(hashProofOfStake), (unsigned char*)&hashProofOfStake);

        // Skip if hash doesn't satisfy the maximum target
        if (hashProofOfStake[7] > nMaxTarget32)
            continue;

        auto bnCoinDayWeight = bnValueIn * GetWeight((int64_t)nInputTxTime, (int64_t)nTimeTx) / COIN / nOneDay;
        auto bnTargetProofOfStake = bnCoinDayWeight * bnTargetPerCoinDay;

        if (bnTargetProofOfStake >= CBigNum(*pnHashProofOfStake))
            solutions.push_back(make_pair(*pnHashProofOfStake, nTimeTx));
    }
}

void KernelWorker::Do()
{
    Do_generic();
}

vector<pair<uint256,uint32_t> >& KernelWorker::GetSolutions()
{
    return solutions;
}

// Scan given kernel for solutions

bool ScanKernelBackward(unsigned char *kernel, uint32_t nBits, uint32_t nInputTxTime, int64_t nValueIn, pair<uint32_t, uint32_t> &SearchInterval, pair<uint256, uint32_t> &solution)
{
    uint256 nTargetPerCoinDay;
    nTargetPerCoinDay.SetCompact(nBits);

    // Get maximum possible target to filter out the majority of obviously insufficient hashes
    auto nMaxTarget = nTargetPerCoinDay * (uint64_t)nValueIn * (uint64_t)nStakeMaxAge / (uint64_t)COIN / (uint64_t)nOneDay;

    SHA256_CTX ctx, workerCtx;
    // Init new sha256 context and update it
    //   with first 24 bytes of kernel
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, kernel, 8 + 16);
    workerCtx = ctx; // save context

    // Search backward in time from the given timestamp
    // Stopping search in case of shutting down
    for (auto nTimeTx=SearchInterval.first; nTimeTx>SearchInterval.second && !fShutdown; nTimeTx--)
    {
        // Complete first hashing iteration
        uint256 hash1;
        SHA256_Update(&ctx, (unsigned char*)&nTimeTx, 4);
        SHA256_Final((unsigned char*)&hash1, &ctx);

        // Restore context
        ctx = workerCtx;

        // Finally, calculate kernel hash
        uint256 hashProofOfStake;
        SHA256((unsigned char*)&hash1, sizeof(hashProofOfStake), (unsigned char*)&hashProofOfStake);

        // Skip if hash doesn't satisfy the maximum target
        if (hashProofOfStake > nMaxTarget)
            continue;

        auto nCoinDayWeight = uint256(nValueIn) * (uint64_t)GetWeight((int64_t)nInputTxTime, (int64_t)nTimeTx) / (uint64_t)COIN / (uint64_t)nOneDay; // TODO: Stop using signed types for value, time, weight and so on, because all these casts are really stupid.
        auto nTargetProofOfStake = nCoinDayWeight * nTargetPerCoinDay;

        if (nTargetProofOfStake >= hashProofOfStake)
        {
            solution = { hashProofOfStake, nTimeTx };
            return true;
        }
    }

    return false;
}
