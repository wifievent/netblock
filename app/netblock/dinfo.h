#pragma once

#include <QString>

#include <string>
#include <mutex>
#include "host.h"

struct DInfo : Host
{
    DInfo() {}
    DInfo(const Host &host) : Host(host) {}

    bool operator == (const DInfo& r) const { return hostId_ == r.hostId_ && mac_ == r.mac_; }

    int hostId_;
    QString oui_;
    bool isConnect_ = false;
};

struct DInfoList : QList<DInfo>
{
    QMutex m_;
};

struct StdDInfo : StdHost
{
    StdDInfo() {}
    StdDInfo(const StdHost &host) : StdHost(host) {}

    bool operator == (const StdDInfo& r) const { return hostId_ == r.hostId_ && mac_ == r.mac_; }
    int hostId_;
    std::string oui_;
    bool isConnect_{false};
};

struct StdDInfoList : std::list<StdDInfo>
{
    std::mutex m_;
};
