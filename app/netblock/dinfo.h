#pragma once

#include <string>
#include <mutex>
#include "host.h"

struct StdDInfo : StdHost
{
    StdDInfo() {}
    StdDInfo(const StdHost &host) : StdHost(host) {}

    bool operator == (const StdDInfo& r) const { return hostId_ == r.hostId_ && mac_ == r.mac_; }
    int hostId_;
    std::string oui_;
    bool isConnect_{false};
};

struct StdDInfoList : std::vector<StdDInfo>
{
    std::mutex m_;
};
