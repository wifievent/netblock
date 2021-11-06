#ifndef DINFO_H
#define DINFO_H

#include <QString>
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

#endif // DINFO_H
