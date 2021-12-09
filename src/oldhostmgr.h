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
struct StdActiveScanThread : std::thread {
    StdActiveScanThread(StdOldHostMgr* ohm, StdHost* host) : std::thread(&StdActiveScanThread::run, this), ohm_(ohm), host_(host) {}
    ~StdActiveScanThread() {}

    void run();

    StdOldHostMgr* ohm_{nullptr};
    StdHost* host_{nullptr};

    std::mutex myMutex_;
    std::condition_variable myCv_;
};

struct StdActiveScanThreadMap: std::map<Mac, StdActiveScanThread*> {
    std::mutex m_;
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
