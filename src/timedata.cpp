#include "timedata.h"
#include "netbase.h"
#include "sync.h"
#include "interface.h"


static CCriticalSection cs_nTimeOffset;
static uint32_t NOVACOIN_TIMEDATA_MAX_SAMPLES = 200;

// Trusted NTP offset or median of NTP samples.
extern int64_t nNtpOffset;

// Median of time samples given by other nodes.
static int64_t nNodesOffset = std::numeric_limits<int64_t>::max();

//
// "Never go to sea with two chronometers; take one or three."
// Our three time sources are:
//  - System clock
//  - Median of other nodes clocks
//  - The user (asking the user to fix the system clock if the first two disagree)
//

// Select time offset:
int64_t GetTimeOffset()
{
    LOCK(cs_nTimeOffset);
    // If NTP and system clock are in agreement within 40 minutes, then use NTP.
    if (std::abs(nNtpOffset) < 40 * 60)
        return nNtpOffset;

    // If not, then choose between median peer time and system clock.
    if (std::abs(nNodesOffset) < 70 * 60)
        return nNodesOffset;

    return 0;
}

int64_t GetNodesOffset()
{
        return nNodesOffset;
}

int64_t GetAdjustedTime()
{
    return GetTime() + GetTimeOffset();
}

void AddTimeData(const CNetAddr& ip, int64_t nTime)
{
    int64_t nOffsetSample = nTime - GetTime();

    LOCK(cs_nTimeOffset);
    // Ignore duplicates
    static std::set<CNetAddr> setKnown;
    if (setKnown.size() == NOVACOIN_TIMEDATA_MAX_SAMPLES)
        return;
    if (!setKnown.insert(ip).second)
        return;

    // Add data
    static CMedianFilter<int64_t> vTimeOffsets(NOVACOIN_TIMEDATA_MAX_SAMPLES,0);
    vTimeOffsets.input(nOffsetSample);
    printf("Added time data, samples %d, offset %+" PRId64 " (%+" PRId64 " minutes)\n", vTimeOffsets.size(), nOffsetSample, nOffsetSample/60);
    if (vTimeOffsets.size() >= 5 && vTimeOffsets.size() % 2 == 1)
    {
        int64_t nMedian = vTimeOffsets.median();
        std::vector<int64_t> vSorted = vTimeOffsets.sorted();
        // Only let other nodes change our time by so much
        if (std::abs(nMedian) < 70 * 60)
        {
            nNodesOffset = nMedian;
        }
        else
        {
            nNodesOffset = std::numeric_limits<int64_t>::max();

            static bool fDone;
            if (!fDone)
            {
                bool fMatch = false;

                // If nobody has a time different than ours but within 5 minutes of ours, give a warning
                for (int64_t nOffset : vSorted)
                    if (nOffset != 0 && std::abs(nOffset) < 5 * 60)
                        fMatch = true;

                if (!fMatch)
                {
                    fDone = true;
                    std::string strMessage = _("Warning: Please check that your computer's date and time are correct! If your clock is wrong NovaCoin will not work properly.");
                    strMiscWarning = strMessage;
                    printf("*** %s\n", strMessage.c_str());
                    uiInterface.ThreadSafeMessageBox(strMessage+" ", std::string("NovaCoin"), CClientUIInterface::OK | CClientUIInterface::ICON_EXCLAMATION);
                }
            }
        }
        if (fDebug) {
            for (int64_t n : vSorted)
                printf("%+" PRId64 "  ", n);
            printf("|  ");
        }
        if (nNodesOffset != std::numeric_limits<int64_t>::max())
            printf("nNodesOffset = %+" PRId64 "  (%+" PRId64 " minutes)\n", nNodesOffset, nNodesOffset/60);
    }
}
