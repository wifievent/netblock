#pragma once

#include <QHash>
#include <GMac>
#include <GIp>

#include <mutex>
#include "mac.h"
#include "ip.h"

struct Host {
    Host() {};
    Host(Mac mac, Ip ip) : mac_(mac), ip_(ip) {}
    Host(Mac mac, Ip ip, QString hostName): mac_(mac), ip_(ip), hostName_(hostName) {}

    Mac mac_;
    Ip ip_;
	QString hostName_;
	QString nickName_;
	qint64 lastAccess_{0};
    QString defaultName();
};

struct HostMap : QHash<Mac, Host> {
    QMutex m_;
};
