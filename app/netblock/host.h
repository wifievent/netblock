#pragma once

#include <QElapsedTimer>
#include <GMac>
#include <GIp>

struct Host {

	Host();
	Host(GMac mac, GIp ip);
	Host(GMac mac, GIp ip, QString name);

	GMac mac_;
	GIp ip_;
	QString name_;
	qint64 lastAccess_;

	static QElapsedTimer& timer() { // singleton
		static QElapsedTimer timer;
		return timer;
	}
};
