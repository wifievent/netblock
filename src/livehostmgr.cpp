#include "livehostmgr.h"

LiveHostMgr::LiveHostMgr(QObject* parent) : GStateObj(parent) {
	QObject::connect(&device_, &GCapture::captured, this, &LiveHostMgr::captured, Qt::DirectConnection);
}

LiveHostMgr::~LiveHostMgr() {
	close();
}

bool LiveHostMgr::doOpen() {
	hosts_.clear();

	if (!device_.open()) {
		SET_ERR(GErr::FAIL, "device_.open() return false");
		return false;
	}
	GIntf* intf = device_.intf();
	Q_ASSERT(intf != nullptr);
	myMac_ = intf->mac();
	qDebug() << "myMac =" << QString(myMac_);

	fs_.device_ = &device_;
	if (!fs_.open())
		return false;

	ohm_.lhm_ = this;
	if (!ohm_.open())
		return false;

	return true;
}

bool LiveHostMgr::doClose() {
	fs_.close();
	ohm_.close();
	device_.close();
	return true;
}

bool LiveHostMgr::processDhcp(GPacket* ethPacket) {
	GUdpHdr* udpHdr = ethPacket->udpHdr_;
	if (udpHdr == nullptr) return false;

	if (!(udpHdr->sport() == 67 || udpHdr->dport() == 67)) return false;

	GBuf dhcp = ethPacket->udpData_;
	if (dhcp.data_ == nullptr) return false;
	if (dhcp.size_ < sizeof(GDhcpHdr)) return false;
	GDhcpHdr* dhcpHdr = PDhcpHdr(dhcp.data_);

	HostMap::iterator newHost = hosts_.end();
	if (dhcpHdr->yourIp() != 0) { // DHCP Offer of DHCP ACK sent from server
		GMac mac = dhcpHdr->clientMac();
		GIp ip = dhcpHdr->yourIp();
		{
			QMutexLocker ml(&hosts_.m_);
			newHost = hosts_.find(mac);
			if (newHost == hosts_.end())
				newHost = hosts_.insert(mac, Host(mac, ip));
			newHost.value().lastAccess_ = et_.elapsed();
		}
		qDebug() << "yourIp";
		emit hostDetected(&newHost.value());
		return true;
	}

	GEthHdr* ethHdr = ethPacket->ethHdr_;
	if (ethHdr == nullptr) return false;
	gbyte* end = ethPacket->buf_.data_ + ethPacket->buf_.size_;
	GDhcpHdr::Option* option = dhcpHdr->first();

	GIp ip = 0;
	QString hostName = "";
	while (true) {
		if (option->type_ == GDhcpHdr::RequestedIpAddress) {
			ip = ntohl(*PIp(option->value()));
		} else if (option->type_ == GDhcpHdr::HostName) {
			for (int i = 0; i < option->len_; i++)
				hostName += *(pchar(option->value()) + i);
		}
		option = option->next();
		if (option == nullptr) break;
		if (pbyte(option) >= end) break;
	}
	if (ip != 0) {
		HostMap::iterator newHost;
		GMac mac = dhcpHdr->clientMac();

		QMutexLocker ml(&hosts_.m_);
		if (hostName == "") {
			Host host(mac, ip);
			newHost = hosts_.find(mac);
			if (newHost == hosts_.end())
				newHost = hosts_.insert(mac, host);
		} else {
			Host host(mac, ip, hostName);
			newHost = hosts_.find(mac);
			if (newHost == hosts_.end())
				newHost = hosts_.insert(mac, host);
			else
				newHost->dhcpName_ = hostName;
			newHost.value().lastAccess_ = et_.elapsed();
		}
		qDebug() << "emit RequestedIpAddress";
		emit hostDetected(&newHost.value());
	}
	return false;
}

void LiveHostMgr::captured(GPacket* packet) {
	GEthPacket* ethPacket = PEthPacket(packet);

	GEthHdr* ethHdr = ethPacket->ethHdr_;
	GMac smac = ethHdr->smac();
	if (smac == myMac_) return;

	if (processDhcp(ethPacket)) return;

	HostMap::iterator newHost = hosts_.end();
	if (ethHdr->type() == GEthHdr::Arp) {
		GArpHdr* arpHdr = ethPacket->arpHdr_;
		GIp sip = arpHdr->sip();
		{
			QMutexLocker ml(&hosts_.m_);
			HostMap::iterator it = hosts_.find(smac);
			if (it == hosts_.end()) {
				Host host(smac, sip);
				host.lastAccess_ = et_.elapsed();
				newHost = hosts_.insert(smac, host);
				qDebug() << "emit Arp";
			} else {
				qDebug() << et_.elapsed(); // gilgil temp 2021.10.24
				it.value().lastAccess_ = et_.elapsed();
			}
		}
	}
	if (newHost != hosts_.end())
		emit hostDetected(&newHost.value());
}

void LiveHostMgr::propLoad(QJsonObject jo) {
	GStateObj::propLoad(jo);
	jo["PcapDevice"] >> device_;
	jo["FullScan"] >> fs_;
	jo["OldHostMgr"] >> ohm_;
}

void LiveHostMgr::propSave(QJsonObject& jo) {
	GStateObj::propSave(jo);
	jo["PcapDevice"] <<  device_;
	jo["FullScan"] << fs_;
	jo["OldHostMgr"] << ohm_;
}
