#pragma once

#include <QMap>
#include <GStateObj>
#include <GThread>
#include <GWaitEvent>

#include <mutex>
#include "host.h"
#include "etharppacket.h"
#include "stateobj.h"

struct StdOldHostMgr;

class testActiveScanThread : std::thread {
public:
    testActiveScanThread(StdOldHostMgr* ohm, Host* host): std::thread(&testActiveScanThread::run, this) {}
    void run();
};

struct StdActiveScanThread : std::thread {
    StdActiveScanThread(StdOldHostMgr* ohm, Host* host) : std::thread(&StdActiveScanThread::run, this), ohm_(ohm), host_(host) {}
    ~StdActiveScanThread();

    void run();

    StdOldHostMgr* ohm_{nullptr};
    Host* host_{nullptr};

    std::mutex myMutex_;
    std::condition_variable myCv_;
};

struct StdActiveScanThreadMap: std::map<Mac, StdActiveScanThread*> {
    QMutex m_;
};

struct StdLiveHostMgr;
struct StdOldHostMgr : StateObj
{
public:
    int checkSleepTime_{10000}; // 10 secs
    int scanStartTimeout_{60000}; // 60 secs
    int sendSleepTime_{1000}; // 1 sec
    int deleteTimeout_{10000}; // 10 secs

public:
    StdOldHostMgr() {}
    ~StdOldHostMgr() override;

protected:
    bool doOpen() override;
    bool doClose() override;

public:
    StdLiveHostMgr* lhm_{nullptr}; // reference

    StdActiveScanThreadMap astm_;

    StdActiveScanThreadMap sastm_;

    std::thread* myThread_;
    std::mutex myMutex_;
    std::condition_variable myCv_;

protected:
    void run();
};

struct OldHostMgr;
struct ActiveScanThread : GThread {
	ActiveScanThread(OldHostMgr* ohm, Host* host);
	~ActiveScanThread() override;
	void run() override;

	OldHostMgr* ohm_{nullptr};
	Host* host_{nullptr};
	GWaitEvent we_;
};

struct ActiveScanThreadMap: QMap<Mac, ActiveScanThread*> {
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

    StdActiveScanThreadMap sastm_;

    std::thread* myThread_;
    std::mutex myMutex_;
    std::condition_variable myCv_;

protected:
	void run();

    struct MyThread: GThread {
        MyThread(QObject *parent);
        ~MyThread();
        void run() override;
    } thread_{this};
};
