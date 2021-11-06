#pragma once

#include <QHash>
#include <GMac>
#include <GIp>

struct Host {
    Host() {};
	Host(GMac mac, GIp ip) : mac_(mac), ip_(ip) {}
	Host(GMac mac, GIp ip, QString hostName): mac_(mac), ip_(ip), hostName_(hostName) {}

	GMac mac_;
	GIp ip_;
	QString hostName_;
	QString nickName_;
	qint64 lastAccess_{0};
};

struct HostMap : QHash<GMac, Host> {
	QMutex m_;
};
