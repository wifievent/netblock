#include "fullscan.h"
#include "etharppacket.h"

#include <glog/logging.h>

bool StdFullScan::doOpen()
{
    if (device_ == nullptr) {
        return false;
    }

    myThread_ = new std::thread(&StdFullScan::run, this);

    return true;
}

bool StdFullScan::doClose() {
    myCv_.notify_all();
    myThread_->join();
    return true;
}

void StdFullScan::run() {
    DLOG(INFO) << "fullscan";

    Intf* intf = device_->intf();
    if (intf == nullptr) {
        qCritical() << "intf is null";
        return;
    }

    Ip myIp = intf->ip();
    Mac myMac = intf->mac();

    Ip begIp = (myIp & intf->mask()) + 1;
    Ip endIp = (myIp | ~intf->mask());

    DLOG(INFO) << "begIp=" << std::string(begIp).data() << "endIp=" << std::string(endIp).data();

    EthArpPacket packet;

    EthHdr* ethHdr = &packet.ethHdr_;
    ethHdr->dmac_ = Mac::broadcastMac();
    ethHdr->smac_ = myMac;
    ethHdr->type_ = htons(EthHdr::Arp);

    ArpHdr* arpHdr = &packet.arpHdr_;
    arpHdr->hrd_ = htons(ArpHdr::ETHER);
    arpHdr->pro_ = htons(EthHdr::Ip4);
    arpHdr->hln_ = Mac::SIZE;
    arpHdr->pln_ = Ip::SIZE;
    arpHdr->op_ = htons(ArpHdr::Request);
    arpHdr->smac_ = myMac;
    arpHdr->sip_ = htonl(myIp);
    arpHdr->tmac_ = Mac::nullMac();

    while (active()) {
        for (Ip ip = begIp; ip <= endIp; ip = ip + 1) {
            arpHdr->tip_ = htonl(ip);
            Packet::Result res = device_->write(Buf(pbyte(&packet), sizeof(packet)));
            if (res != Packet::Ok) {
                DLOG(WARNING) << "device_->write return" << int(res);
            }

            std::unique_lock<std::mutex> lock(myMutex_);
            if(myCv_.wait_for(lock,std::chrono::milliseconds(sendSleepTime_)) == std::cv_status::no_timeout) break;
        }
        if (!active()) break;

        std::unique_lock<std::mutex> lock(myMutex_);
        if(myCv_.wait_for(lock, std::chrono::milliseconds(rescanSleepTime_)) == std::cv_status::no_timeout) break;
    }

    DLOG(INFO) << "end";
}
