#pragma once

#include <QHash>
#include <GMac>
#include <GIp>

struct Host {
	Host();
	Host(GMac mac, GIp ip) : mac_(mac), ip_(ip) {}
	Host(GMac mac, GIp ip, QString dhcpName): mac_(mac), ip_(ip), dhcpName_(dhcpName) {}

	GMac mac_;
	GIp ip_;
	QString dhcpName_;
	QString nickName_;
	qint64 lastAccess_{0};
};

struct HostMap : QHash<GMac, Host> {
	QMutex m_;
};
