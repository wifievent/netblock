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

#include "pcapdevice.h"
#include "dhcphdr.h"

struct G_EXPORT LiveHostMgr : GStateObj {
    Q_OBJECT

public:
    LiveHostMgr(QObject* parent, PcapDevice* pcapDevice);
    ~LiveHostMgr() override;

protected:
    bool doOpen() override;
    bool doClose() override;

public:
    HostMap hosts_;
    PcapDevice* device_{nullptr};
    FullScan fs_;
    OldHostMgr ohm_;
    QElapsedTimer et_;

    Intf* intf_{nullptr};
    Mac myMac_{Mac::nullMac()};

protected:
    bool processDhcp(Packet* packet, Mac* mac, Ip* ip, QString* hostName);
    bool processArp(EthHdr* ethHdr, ArpHdr* arpHdr, Mac* mac, Ip* ip);
    bool processIp(EthHdr* ethHdr, IpHdr* ipHdr, Mac* mac, Ip* ip);

public slots:
    void captured(Packet* packet);

signals:
    void hostDetected(Host* host);
    void hostDeleted(Host* host);

public:
    void propLoad(QJsonObject jo) override;
    void propSave(QJsonObject& jo) override;
};
