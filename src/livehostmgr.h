#pragma once

#include <QMutexLocker>
#include <QMutex>
#include <QVector>
#include <GJson>
#include <GPcapDevice>
#include <GStateObj>
#include <GDhcpHdr>
#include "host.h"
#include "fullscan.h"

struct HostVector : protected QVector<Host> {
public:
	QMutex m_;

	void clear() { QVector<Host>::clear(); }
	Host* find(GMac mac);
	Host* add(Host host);
};

struct G_EXPORT LiveHostMgr : GStateObj {
	Q_OBJECT

public:
	LiveHostMgr(QObject* parent = nullptr);
	~LiveHostMgr() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	HostVector hosts_;
	GPcapDevice device_;
	GMac myMac_{GMac::nullMac()};

	FullScan fs_;

protected:
	bool processDhcp(GPacket* ethPacket);

public slots:
	void captured(GPacket* packet);

signals:
	void hostDetected(Host* host);
	void hostDeleted(Host* host);

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
