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
    if (dhcpHdr->yourIp() != 0) { // DHCP Offer of DHCP ACK sent from server
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

    if (!detected) return;

    if (hostName != "") {
        StdHost host(mac, ip, hostName);
        emit hostDetected(&host);
    }

    StdHostMap::iterator newHost = hosts_.end();
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        std::lock_guard<std::mutex> lock(hosts_.m_);
        StdHostMap::iterator it = hosts_.find(mac);
        if (it == hosts_.end()) {
            StdHost host(mac, ip, hostName);
            host.lastAccess_ = now.tv_sec;
            hosts_.insert(std::pair<Mac, StdHost>(mac, host));
        } else {
            it->second.lastAccess_ = now.tv_sec;
        }
    }
    if (newHost != hosts_.end())
        emit hostDetected(&newHost->second);
}

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
