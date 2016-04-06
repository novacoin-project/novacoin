#include "kernelrecord.h"

#include "wallet.h"
#include "base58.h"

using namespace std;

bool KernelRecord::showTransaction(const CWalletTx &wtx)
{
    if (wtx.IsCoinBase())
    {
        if (wtx.GetDepthInMainChain() < 2)
        {
            return false;
        }
    }

    if(!wtx.IsTrusted())
    {
        return false;
    }

    return true;
}

/*
 * Decompose CWallet transaction to model kernel records.
 */
vector<KernelRecord> KernelRecord::decomposeOutput(const CWallet *wallet, const CWalletTx &wtx)
{
    vector<KernelRecord> parts;
    auto nTime = wtx.GetTxTime();
    auto hash = wtx.GetHash();
    auto mapValue = wtx.mapValue;
    auto nDayWeight = (min((GetAdjustedTime() - nTime), (int64_t)(nStakeMaxAge+nStakeMinAge)) - nStakeMinAge); // DayWeight * 86400, чтобы был
                                                                                                              // правильный расчёт CoinAge
    if (showTransaction(wtx))
    {
        for (unsigned int nOut = 0; nOut < wtx.vout.size(); nOut++)
        {
            CTxOut txOut = wtx.vout[nOut];
            if( wallet->IsMine(txOut) ) {
                CTxDestination address;
                string addrStr;

                auto coinAge = max( (txOut.nValue * nDayWeight) / (COIN * nOneDay), (int64_t)0);

                if (ExtractDestination(txOut.scriptPubKey, address))
                {
                    // Sent to Bitcoin Address
                    addrStr = CBitcoinAddress(address).ToString();
                }
                else
                {
                    // Sent to IP, or other non-address transaction like OP_EVAL
                    addrStr = mapValue["to"];
                }

                parts.push_back(KernelRecord(hash, nTime, addrStr, txOut.nValue, wtx.IsSpent(nOut), coinAge));
            }
        }
    }

    return parts;
}

string KernelRecord::getTxID()
{
    return hash.ToString() + strprintf("-%03d", idx);
}

int64_t KernelRecord::getAge() const
{
    return (GetAdjustedTime() - nTime) / nOneDay;
}

uint64_t KernelRecord::getCoinDay() const
{
    auto nWeight = GetAdjustedTime() - nTime - nStakeMinAge;
    if( nWeight <  0)
        return 0;
    nWeight = min(nWeight, (int64_t)nStakeMaxAge);
    return (nValue * nWeight ) / (COIN * nOneDay);
}

int64_t KernelRecord::getPoSReward(int nBits, int minutes)
{
    auto nWeight = GetAdjustedTime() - nTime + minutes * 60;
    if( nWeight <  nStakeMinAge)
        return 0;
    auto coinAge = (nValue * nWeight ) / (COIN * nOneDay);
    return GetProofOfStakeReward(coinAge, nBits, GetAdjustedTime() + minutes * 60);
}

double KernelRecord::getProbToMintStake(double difficulty, int timeOffset) const
{
    //double maxTarget = pow(static_cast<double>(2), 224);
    //double target = maxTarget / difficulty;
    //int dayWeight = (min((GetAdjustedTime() - nTime) + timeOffset, (int64_t)(nStakeMinAge+nStakeMaxAge)) - nStakeMinAge) / 86400;
    //uint64_t coinAge = max(nValue * dayWeight / COIN, (int64_t)0);
    //return target * coinAge / pow(static_cast<double>(2), 256);
    auto Weight = (min((GetAdjustedTime() - nTime) + timeOffset, (int64_t)(nStakeMinAge+nStakeMaxAge)) - nStakeMinAge);
    auto coinAge = max(nValue * Weight / (COIN * nOneDay), (int64_t)0);
    return coinAge / (pow(static_cast<double>(2),32) * difficulty);
}

double KernelRecord::getProbToMintWithinNMinutes(double difficulty, int minutes)
{
    if(difficulty != prevDifficulty || minutes != prevMinutes)
    {
        double prob = 1;
        double p;
        int d = minutes / (60 * 24); // Number of full days
        int m = minutes % (60 * 24); // Number of minutes in the last day
        int i, timeOffset;

        // Probabilities for the first d days
        for(i = 0; i < d; i++)
        {
            timeOffset = i * nOneDay;
            p = pow(1 - getProbToMintStake(difficulty, timeOffset), nOneDay);
            prob *= p;
        }

        // Probability for the m minutes of the last day
        timeOffset = d * nOneDay;
        p = pow(1 - getProbToMintStake(difficulty, timeOffset), 60 * m);
        prob *= p;

        prob = 1 - prob;
        prevProbability = prob;
        prevDifficulty = difficulty;
        prevMinutes = minutes;
    }
    return prevProbability;
}
