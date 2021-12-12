#pragma once

#include <QObject>
#include "host.h"
#include "fullscan.h"
#include "oldhostmgr.h"

#include "arpspoof.h"
#include "dhcphdr.h"


struct StdLiveHostMgr : StateObj {

public:
    StdLiveHostMgr(ArpSpoof* arpDevice): device_(arpDevice) {};
    ~StdLiveHostMgr() override {};

protected:
    bool doOpen() override;
    bool doClose() override;

public:
    StdHostMap hosts_;
    ArpSpoof* device_{nullptr};
    StdFullScan fs_;
    StdOldHostMgr ohm_;

    Intf* intf_{nullptr};
    Mac myMac_{Mac::nullMac()};

protected:
    bool processDhcp(Packet* packet, Mac* mac, Ip* ip, std::string* hostName);
    bool processArp(EthHdr* ethHdr, ArpHdr* arpHdr, Mac* mac, Ip* ip);
    bool processIp(EthHdr* ethHdr, IpHdr* ipHdr, Mac* mac, Ip* ip);

public:
    void captured(Packet* packet);

    void hostDetected(StdHost* host);
    void hostDeleted(StdHost* host);

public:
    void load(Json::Value& json) override;
    void save(Json::Value& json) override;
};
