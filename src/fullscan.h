#pragma once

#include <GStateObj>

struct G_EXPORT FullScan : GStateObj {
	Q_OBJECT
	Q_PROPERTY(int sendInterval MEMBER sendInterval_)
	Q_PROPERTY(int fullScanSleepTime MEMBER rescanInterval_)

	int sendInterval_{10}; // 10 msec
	int rescanInterval_{60000}; // 10 minutes
};
