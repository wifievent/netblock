#include "fullscan.h"
#include "etharppacket.h"

FullScan::FullScan(QObject* parent) : GStateObj(parent) {
}

FullScan::~FullScan() {
	close();
}

bool FullScan::doOpen() {
	if (device_ == nullptr) {
		SET_ERR(GErr::OBJECT_IS_NULL, "device_ is null");
		return false;
	}

    myThread_ = new std::thread(&FullScan::run, this);

	return true;
}

bool FullScan::doClose() {
    myCv_.notify_all();
    myThread_->join();
	return true;
}

void FullScan::run() {
	qDebug() << "beg";

    Intf* intf = device_->intf();
	if (intf == nullptr) {
		qCritical() << "intf is null";
		return;
	}

    Ip myIp = intf->ip();
    Mac myMac = intf->mac();

    Ip begIp = (myIp & intf->mask()) + 1;
    Ip endIp = (myIp | ~intf->mask());

    qDebug() << QString("begIp=%1 endIp=%2").arg(QString(std::string(begIp).data()), QString(std::string(endIp).data()));

	EthArpPacket packet;

    EthHdr* ethHdr = &packet.ethHdr_;
    ethHdr->dmac_ = Mac::broadcastMac();
	ethHdr->smac_ = myMac;
	ethHdr->type_ = htons(GEthHdr::Arp);

    ArpHdr* arpHdr = &packet.arpHdr_;
	arpHdr->hrd_ = htons(GArpHdr::ETHER);
	arpHdr->pro_ = htons(GEthHdr::Ip4);
	arpHdr->hln_ = GMac::SIZE;
	arpHdr->pln_ = GIp::SIZE;
	arpHdr->op_ = htons(GArpHdr::Request);
	arpHdr->smac_ = myMac;
	arpHdr->sip_ = htonl(myIp);
    arpHdr->tmac_ = Mac::nullMac();

	while (active()) {
        for (Ip ip = begIp; ip <= endIp; ip = ip + 1) {
			arpHdr->tip_ = htonl(ip);
            Packet::Result res = device_->write(Buf(pbyte(&packet), sizeof(packet)));
            if (res != Packet::Ok) {
                qWarning() << QString("device_->write return %1").arg(int(res));
            }
            {
                std::unique_lock<std::mutex> lock(myMutex_);
                if(myCv_.wait_for(lock,std::chrono::milliseconds(sendSleepTime_)) == std::cv_status::no_timeout) break;
            }
		}
		if (!active()) break;
        {
            std::unique_lock<std::mutex> lock(myMutex_);
            if(myCv_.wait_for(lock, std::chrono::milliseconds(rescanSleepTime_)) == std::cv_status::no_timeout) break;
        }
	}

	qDebug() << "end";
}
