#ifndef BITCOIN_TIMESTAMPS_H
#define BITCOIN_TIMESTAMPS_H

static const unsigned int TARGETS_SWITCH_TIME = 1374278400; // Saturday, 20-Jul-2013 00:00:00 UTC
static const unsigned int CHECKLOCKTIMEVERIFY_SWITCH_TIME = 1461110400; // Wednesday, 20-Apr-16 00:00:00 UTC
// Threshold for nLockTime: below this value it is interpreted as block number, otherwise as UNIX timestamp.
static const unsigned int LOCKTIME_THRESHOLD = 500000000; // Tue Nov  5 00:53:20 1985 UTC
#endif
