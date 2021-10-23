#pragma once

#include <QObject>
#include <QDebug>
#include "host.h"

struct Obj : public QObject {
	Q_OBJECT

public:
	Obj(QObject* parent = nullptr) : QObject(parent) {}
	~Obj() override {}

public slots:
	void processHostDetected(Host* host) {
		qDebug() << QString("host detected %1 %2 %3 %4").arg(QString(host->mac_), QString(host->ip_), host->dhcpName_, host->nickName_);
	}

	void processHostDeleted(Host* host) {
		qDebug() << QString("host deleted %1 %2 %3 %4").arg(QString(host->mac_), QString(host->ip_), host->dhcpName_, host->nickName_);
	}
};
