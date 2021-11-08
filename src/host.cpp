#include "host.h"

QString Host::defaultName() {
    if(!nickName_.isNull())
        return nickName_;
    if(!hostName_.isNull())
        return hostName_;
    return QString(ip_);
}
