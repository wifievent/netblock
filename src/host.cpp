#include "host.h"

std::string StdHost::defaultName()
{
    if(!nickName_.empty())
        return nickName_;
    if(!hostName_.empty())
        return hostName_;
    return std::string(ip_);
}
