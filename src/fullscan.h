#pragma once

#include <thread>
#include <mutex>
#include "pcapdevice.h"

#include "appjson.h"

struct StdFullScan : StateObj
{

    int sendSleepTime_{50}; // 50 msecs
    int rescanSleepTime_{600000}; // 10 minutes

public:
    StdFullScan() {}
    ~StdFullScan() {close();}

    PcapDevice* device_{nullptr}; // reference

    void load(Json::Value& json) override;
    void save(Json::Value& json) override;

protected:
    bool doOpen() override;
    bool doClose() override;

protected:
    void run();

    std::thread* myThread_;
    std::mutex myMutex_;
    std::condition_variable myCv_;
};
