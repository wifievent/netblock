#include "livehostmgr.h"

#include <glog/logging.h>

bool StdLiveHostMgr::doOpen()
{
    hosts_.clear();

    intf_ = device_->intf();
    myMac_ = intf_->mac();

    DLOG(INFO) << "myMac="<<std::string(myMac_).data();

    fs_.device_ = device_;
    if (!fs_.open())
        return false;

    ohm_.lhm_ = this;
    if (!ohm_.open())
        return false;

    return true;
}

bool StdLiveHostMgr::doClose() {
    fs_.close();
    ohm_.close();
    return true;
}

bool StdLiveHostMgr::processDhcp(Packet* packet, Mac* mac, Ip* ip, std::string* hostName) {
    UdpHdr* udpHdr = packet->udpHdr_;
    if (udpHdr == nullptr) return false;

    if (!(udpHdr->sport() == 67 || udpHdr->dport() == 67)) return false;

    Buf dhcp = packet->udpData_;
    if (dhcp.data_ == nullptr) return false;
    if (dhcp.size_ < sizeof(DhcpHdr)) return false;
    DhcpHdr* dhcpHdr = PDhcpHdr(dhcp.data_);

    bool ok = false;
    if (dhcpHdr->yourIp() == Ip::nullIp()) { // DHCP Offer of DHCP ACK sent from server

    }
    else
    {
        *mac = dhcpHdr->clientMac();
        *ip = dhcpHdr->yourIp();
        ok = true;
    }

    EthHdr* ethHdr = packet->ethHdr_;
    if (ethHdr == nullptr) return false;
    pbyte end = packet->buf_.data_ + packet->buf_.size_;
    DhcpHdr::Option* option = dhcpHdr->first();
    while (true) {
        if (option->type_ == DhcpHdr::RequestedIpAddress) {
            *ip = ntohl(*(Ip*)(option->value()));
            *mac = ethHdr->smac();
            ok = true;
        } else if (option->type_ == DhcpHdr::HostName) {
            for (int i = 0; i < option->len_; i++)
                *hostName += *(pchar(option->value()) + i);
        }
        option = option->next();
        if (option == nullptr) break;
        if (pbyte(option) >= end) break;
    }
    return ok;
}

bool StdLiveHostMgr::processArp(EthHdr* ethHdr, ArpHdr* arpHdr, Mac* mac, Ip* ip) {
    if (ethHdr->smac() != arpHdr->smac()) {
        DLOG(INFO) << "ARP spoofing detected"
                   << std::string(ethHdr->smac()).data()
                   << std::string(arpHdr->smac()).data()
                   << std::string(arpHdr->sip()).data();
        return false;
    }

    *mac = arpHdr->smac();
    *ip = arpHdr->sip();
    return true;
}

bool StdLiveHostMgr::processIp(EthHdr* ethHdr, IpHdr* ipHdr, Mac* mac, Ip* ip) {
    Ip sip = ipHdr->sip();
    if (!intf_->isSameLanIp(sip)) return false;

    *mac = ethHdr->smac();
    *ip = sip;
    return true;
}

void StdLiveHostMgr::captured(Packet* packet) {

    DLOG(INFO) << "LHM captured";
    Mac mac;
    Ip ip;
    std::string hostName;

    EthHdr* ethHdr = packet->ethHdr_;
    if (ethHdr == nullptr) return;

    mac = ethHdr->smac();
    if (mac == myMac_) return;


    bool detected = false;
    IpHdr* ipHdr = packet->ipHdr_;
    if (ipHdr != nullptr) {
        if (processDhcp(packet, &mac, &ip, &hostName))
            detected = true;
        else if (processIp(ethHdr, ipHdr, &mac, &ip))
            detected = true;
    }

    ArpHdr* arpHdr = packet->arpHdr_;
    if (arpHdr != nullptr) {
        if (processArp(ethHdr, arpHdr, &mac, &ip))
            detected = true;
    }

    DLOG(INFO) << "host detected: " << detected << " host name: " << hostName;

    if (!detected) return;

    if (hostName != "") {
        StdHost host(mac, ip, hostName);
        emit hostDetected(&host);
    }

    std::pair<StdHostMap::iterator, bool> newHost;
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        std::lock_guard<std::mutex> lock(hosts_.m_);
        StdHostMap::iterator it = hosts_.find(mac);
        DLOG(INFO) << "new mac: " << std::string(mac).data() << "bool: " << (it == hosts_.end()) << "host size: " << hosts_.size();
        if (it == hosts_.end()) {
            StdHost host(mac, ip, hostName);
            host.lastAccess_ = now.tv_sec;
            newHost = hosts_.insert(std::pair<Mac, StdHost>(mac, host));
        } else {
            it->second.lastAccess_ = now.tv_sec;
        }
    }

    DLOG(INFO) << "newHost second" << newHost.second;

    if (newHost.second)
        emit hostDetected(&newHost.first->second);
}

//void StdLiveHostMgr::hostDetected(StdHost* host)
//{
//    DLOG(INFO) << "Detected" << host->defaultName();
//    StdDInfo tmp(*host);
//    tmp.isConnect_ = true;

//    std::string ouiQuery("SELECT organ FROM oui WHERE substr(mac, 1, 8) = substr(':mac', 1, 8)");
//    ouiQuery.replace(ouiQuery.find(":mac"), std::string(":mac").length(), std::string(tmp.mac_));
//    std::list<DataList> dl = ouiConnect_->selectQuery(ouiQuery);

//    if(dl.size() > 0)
//    {
//        for(DataList data: dl)
//        {
//            tmp.oui_ = data.argv_[0];
//        }
//    }

//    std::string query("INSERT INTO host(mac, last_ip, host_name, nick_name, oui) VALUES(':mac', ':last_ip', ':host_name', ':nick_name', ':oui')");
//    query.replace(query.find(":mac"), std::string(":mac").length(), std::string(tmp.mac_));
//    query.replace(query.find(":last_ip"), std::string(":last_ip").length(), std::string(tmp.ip_));
//    query.replace(query.find(":host_name"), std::string(":host_name").length(), tmp.hostName_);
//    query.replace(query.find(":nick_name"), std::string(":nick_name").length(), tmp.nickName_);
//    query.replace(query.find(":oui"), std::string(":oui").length(), tmp.oui_);
//    nbConnect_->sendQuery(query);
//}
//void StdLiveHostMgr::hostDeleted(StdHost* host)
//{

//    DLOG(INFO) << "deleted" << host->defaultName();
//}

void StdLiveHostMgr::load(Json::Value &json)
{
    fs_ << json["FullScan"];
    ohm_ << json["OldHostMgr"];
}

void StdLiveHostMgr::save(Json::Value &json)

{
    fs_ >> json["FullScan"];
    ohm_ >> json["OldHostMgr"];
}
