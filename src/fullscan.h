#pragma once

#include <thread>
#include <mutex>

#include "appjson.h"
#include "arpspoof.h"

struct StdFullScan : StateObj
{

    int sendSleepTime_{50}; // 50 msecs
    int rescanSleepTime_{6000}; // 10 minutes

public:
    StdFullScan() {}
    ~StdFullScan() {close();}

    ArpSpoof* device_{nullptr}; // reference

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
