// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "protocol.h"
#include "util.h"
#include "netbase.h"

using namespace std;

// Network ID, previously known as pchMessageStart
uint32_t nNetworkID = 0xe5e9e8e4;

static const vector<const char*> vpszTypeName = { "ERROR", "tx", "block" };

CMessageHeader::CMessageHeader() : nNetworkID(::nNetworkID), nMessageSize(numeric_limits<uint32_t>::max()), nChecksum(0)
{
    fill(begin(pchCommand), end(pchCommand), '\0');
    pchCommand[1] = 1;
}

CMessageHeader::CMessageHeader(const char* pszCommand, unsigned int nMessageSizeIn) : nNetworkID(::nNetworkID), nMessageSize(nMessageSizeIn), nChecksum(0)
{
    strncpy(pchCommand, pszCommand, COMMAND_SIZE);
}

string CMessageHeader::GetCommand() const
{
    if (pchCommand[COMMAND_SIZE-1] == 0)
        return string(pchCommand, pchCommand + strlen(pchCommand));
    else
        return string(pchCommand, pchCommand + COMMAND_SIZE);
}

bool CMessageHeader::IsValid() const
{
    // Check start string
    if (nNetworkID != ::nNetworkID)
        return false;

    // Check the command string for errors
    for (const char* p1 = pchCommand; p1 < pchCommand + COMMAND_SIZE; p1++)
    {
        if (*p1 == 0)
        {
            // Must be all zeros after the first zero
            for (; p1 < pchCommand + COMMAND_SIZE; p1++)
                if (*p1 != 0)
                    return false;
        }
        else if (*p1 < ' ' || *p1 > 0x7E)
            return false;
    }

    // Message size
    if (nMessageSize > MAX_SIZE)
    {
        printf("CMessageHeader::IsValid() : (%s, %u bytes) nMessageSize > MAX_SIZE\n", GetCommand().c_str(), nMessageSize);
        return false;
    }

    return true;
}

CAddress::CAddress() : CService(), nServices(NODE_NETWORK), nTime(100000000), nLastTry(0) { }
CAddress::CAddress(CService ipIn, uint64_t nServicesIn) : CService(ipIn), nServices(nServicesIn), nTime(100000000), nLastTry(0) { }
CInv::CInv() { }
CInv::CInv(int typeIn, const uint256& hashIn) : type(typeIn), hash(hashIn) { }
CInv::CInv(const string& strType, const uint256& hashIn) : hash(hashIn)
{
    unsigned int i;
    for (i = 1; i < vpszTypeName.size(); ++i)
    {
        if (strType.compare(vpszTypeName[i]) == 0) {
            type = i;
            break;
        }
    }
    if (i == vpszTypeName.size())
        throw out_of_range(strprintf("CInv::CInv(string, uint256) : unknown type '%s'", strType.c_str()));
}

bool operator<(const CInv& a, const CInv& b)
{
    return (a.type < b.type || (a.type == b.type && a.hash < b.hash));
}

bool CInv::IsKnownType() const
{
    return (type >= 1 && type < (int)vpszTypeName.size());
}

const char* CInv::GetCommand() const
{
    if (!IsKnownType())
        throw out_of_range(strprintf("CInv::GetCommand() : type=%d unknown type", type));
    return vpszTypeName[type];
}

string CInv::ToString() const
{
    return strprintf("%s %s", GetCommand(), hash.ToString().substr(0,20).c_str());
}
