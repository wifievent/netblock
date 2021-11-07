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
    LiveHostMgr(QObject* parent = nullptr);
    ~LiveHostMgr() override;

protected:
    bool doOpen() override;
    bool doClose() override;

public:
    HostMap hosts_;
    GPcapDevice device_;
    FullScan fs_;
    OldHostMgr ohm_;
    QElapsedTimer et_;

    GMac myMac_{GMac::nullMac()};

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
