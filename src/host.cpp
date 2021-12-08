#include "host.h"

QString Host::defaultName() {
    if(!nickName_.isNull())
        return nickName_;
    if(!hostName_.isNull())
        return hostName_;
    return QString(std::string(ip_).data());
}

std::string StdHost::defaultName()
{
    if(!nickName_.empty())
        return nickName_;
    if(!hostName_.empty())
        return hostName_;
    return std::string(ip_);
}
