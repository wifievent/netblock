#pragma once

#include <QElapsedTimer>
#include <GMac>
#include <GIp>

struct Host {
	Host();
	Host(GMac mac, GIp ip);
	Host(GMac mac, GIp ip, QString dhcpName);

	GMac mac_;
	GIp ip_;
	QString dhcpName_;
	QString nickName_;
	qint64 lastAccess_;

	static QElapsedTimer& timer() { // singleton
		static QElapsedTimer timer;
		return timer;
	}
};
