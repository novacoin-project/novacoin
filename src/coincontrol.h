#ifndef COINCONTROL_H
#define COINCONTROL_H

#include "base58.h"

class COutPoint;

/** Coin Control Features. */
class CCoinControl
{
public:
    CBitcoinAddress destChange;

    CCoinControl();
        
    void SetNull();
    bool HasSelected() const;
    bool IsSelected(const uint256& hash, unsigned int n) const;
    void Select(COutPoint& output);
    void UnSelect(COutPoint& output);
    void UnSelectAll();
    void ListSelected(std::vector<COutPoint>& vOutpoints);
        
private:
    std::set<COutPoint> setSelected;

};

#endif // COINCONTROL_H
