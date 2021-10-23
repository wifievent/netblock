#pragma once

#include <QMap>
#include <GStateObj>
#include <GThread>
#include <GWaitEvent>
#include "host.h"
#include "etharppacket.h"

struct OldHostMgr;
struct ActiveScanThread : GThread {
	ActiveScanThread(QObject* owner, Host* host);
	~ActiveScanThread() override;
	void run() override;

	OldHostMgr* ohm_{nullptr};
	Host* host_{nullptr};
	GWaitEvent we_;
};

struct ActiveScanThreadMap: QMap<GMac, ActiveScanThread*> {
	QMutex m_;
};

struct LiveHostMgr;
struct OldHostMgr : GStateObj {
	Q_OBJECT

	Q_PROPERTY(int checkSleepTime MEMBER checkSleepTime_)
	Q_PROPERTY(int scanStartTimeout MEMBER scanStartTimeout_)
	Q_PROPERTY(int sendSleepTime MEMBER sendSleepTime_)
	Q_PROPERTY(int deleteTimeout MEMBER deleteTimeout_)

public:
	int checkSleepTime_{10000}; // 10 secs
	int scanStartTimeout_{60000}; // 60 secs
	int sendSleepTime_{1000}; // 1 sec
	int deleteTimeout_{10000}; // 10 secs

public:
	OldHostMgr(QObject* parent = nullptr);
	~OldHostMgr() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	LiveHostMgr* lhm_{nullptr}; // reference

	ActiveScanThreadMap astm_;
	GWaitEvent we_;

protected:
	void run();

	struct MyThread: GThread {
		MyThread(QObject *parent);
		~MyThread();
		void run() override;
	} thread_{this};
};
