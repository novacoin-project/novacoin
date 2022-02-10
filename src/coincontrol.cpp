#include "coincontrol.h"
#include "wallet.h"

CCoinControl::CCoinControl()
{
    SetNull();
}

void CCoinControl::SetNull()
{
    destChange = CBitcoinAddress();
    setSelected.clear();
}

bool CCoinControl::HasSelected() const
{
    return (setSelected.size() > 0);
}

bool CCoinControl::IsSelected(const uint256 &hash, unsigned int n) const
{
    COutPoint outpt(hash, n);
    return (setSelected.count(outpt) > 0);
}

void CCoinControl::Select(COutPoint &output)
{
    setSelected.insert(output);
}

void CCoinControl::UnSelect(COutPoint &output)
{
    setSelected.erase(output);
}

void CCoinControl::UnSelectAll()
{
    setSelected.clear();
}

void CCoinControl::ListSelected(std::vector<COutPoint> &vOutpoints)
{
    vOutpoints.assign(setSelected.begin(), setSelected.end());
}
