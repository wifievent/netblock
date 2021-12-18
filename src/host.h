#pragma once

#include <QObject>

#include <string>
#include <map>
#include <mutex>
#include "mac.h"
#include "ip.h"

struct StdHost
{
public:
    StdHost() {}
    StdHost(Mac mac, Ip ip) : mac_(mac), ip_(ip) {}
    StdHost(Mac mac, Ip ip, std::string hostName): mac_(mac), ip_(ip), hostName_(hostName) {}

    Mac mac_;
    Ip ip_;
    std::string hostName_;
    std::string nickName_;
    long lastAccess_{0};
    std::string defaultName();
};

struct StdHostMap : std::map<Mac, StdHost>
{
    std::mutex m_;
};
