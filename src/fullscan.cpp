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

	thread_.start();
	return true;
}

bool FullScan::doClose() {
	we_.wakeAll();
	if (!thread_.wait() ) {
		qCritical() << "thread_.wait return false";
	}
	return true;
}

void FullScan::run() {
	qDebug() << "beg";

	GIntf* intf = device_->intf();
	Q_ASSERT(intf != nullptr);

	GIp myIp = intf->ip();
	GMac myMac = intf->mac();

	GIp begIp = (myIp & intf->mask()) + 1;
	GIp endIp = (myIp | ~intf->mask());
	qDebug() << QString("begIp=%1 endIp=%2").arg(QString(begIp), QString(endIp));

	EthArpPacket arpPacket;

	GEthHdr* ethHdr = &arpPacket.ethHdr_;
	ethHdr->dmac_ = GMac::broadcastMac();
	ethHdr->smac_ = myMac;
	ethHdr->type_ = htons(GEthHdr::Arp);

	GArpHdr* arpHdr = &arpPacket.arpHdr_;
	arpHdr->hrd_ = htons(GArpHdr::ETHER);
	arpHdr->pro_ = htons(GEthHdr::Ip4);
	arpHdr->hln_ = GMac::SIZE;
	arpHdr->pln_ = GIp::SIZE;
	arpHdr->op_ = htons(GArpHdr::Request);
	arpHdr->smac_ = myMac;
	arpHdr->sip_ = htonl(myIp);
	arpHdr->tmac_ = GMac::nullMac();

	GWaitEvent we;
	while (active()) {
		for (GIp ip = begIp; ip <= endIp; ip = ip + 1) {
			arpHdr->tip_ = htonl(ip);
			GPacket::Result res = device_->write(GBuf(pbyte(&arpPacket), sizeof(arpPacket)));
			if (res != GPacket::Ok) {
				qWarning() << QString("device_->write return %1 %2").arg(int(res)).arg(device_->err->msg());
			}
			if (we_.wait(sendSleepTime_)) break;
		}
		if (!active()) break;
		if (we_.wait(rescanSleepTime_)) break;
	}

	qDebug() << "end";
}
