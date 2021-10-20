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
	void processhostAdded(Host* host) {
		qDebug() << QString("host added %1 %2 %3").arg(QString(host->mac_), QString(host->ip_), QString(host->name_));
	}

	void processhostDeleted(Host* host) {
		qDebug() << QString("host deleted %1 %2 %3").arg(QString(host->mac_), QString(host->ip_), QString(host->name_));
	}
};
