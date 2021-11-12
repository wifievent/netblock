#pragma once

#include <QMutexLocker>
#include <QMutex>
#include <GJson>
#include <GPcapDevice>
#include <GStateObj>
#include <GDhcpHdr>
#include "host.h"
#include "fullscan.h"
#include "oldhostmgr.h"

struct G_EXPORT LiveHostMgr : GStateObj {
    Q_OBJECT

public:
	LiveHostMgr(QObject* parent, GPcapDevice* pcapDevice);
    ~LiveHostMgr() override;

protected:
    bool doOpen() override;
    bool doClose() override;

public:
    HostMap hosts_;
	GPcapDevice* device_{nullptr};
    FullScan fs_;
    OldHostMgr ohm_;
    QElapsedTimer et_;

	GIntf* intf_{nullptr};
    GMac myMac_{GMac::nullMac()};

protected:
	bool processDhcp(GPacket* packet, GMac* mac, GIp* ip, QString* hostName);
	bool processArp(GEthHdr* ethHdr, GArpHdr* arpHdr, GMac* mac, GIp* ip);
	bool processIp(GEthHdr* ethHdr, GIpHdr* ipHdr, GMac* mac, GIp* ip);

public slots:
    void captured(GPacket* packet);

signals:
    void hostDetected(Host* host);
    void hostDeleted(Host* host);

public:
    void propLoad(QJsonObject jo) override;
    void propSave(QJsonObject& jo) override;
};
