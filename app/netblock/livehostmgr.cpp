#include "livehostmgr.h"

Host* HostVector::find(GMac mac) {
	for (Host& host : *this) {
		if (host.mac_ == mac)
			return &host;
	}
	return nullptr;
}

Host* HostVector::add(Host host) {
	push_back(host);
	return find(host.mac_);
}

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
	myIntf_ = device_.intf();
	Q_ASSERT(myIntf_ != nullptr);
	myMac_ = myIntf_->mac();
	qDebug() << "myMac =" << QString(myMac_);

	return true;
}

bool LiveHostMgr::doClose() {
	bool res = device_.close();
	return res;
}

bool LiveHostMgr::processDhcp(GPacket* ethPacket) {
	GUdpHdr* udpHdr = ethPacket->udpHdr_;
	if (udpHdr == nullptr) return false;

	if (!(udpHdr->sport() == 67 || udpHdr->dport() == 67)) return false;

	GBuf dhcp = ethPacket->udpData_;
	if (dhcp.data_ == nullptr) return false;
	if (dhcp.size_ < sizeof(GDhcpHdr)) return false;
	GDhcpHdr* dhcpHdr = PDhcpHdr(dhcp.data_);

	Host* newHost = nullptr;
	if (dhcpHdr->yourIp() != 0) { // DHCP Offer of DHCP ACK sent from server
		GMac mac = dhcpHdr->clientMac();
		GIp ip = dhcpHdr->yourIp();

		{
			QMutexLocker locker(&hosts_.m_);
			newHost = hosts_.find(mac);
			if (newHost == nullptr)
				newHost = hosts_.add(Host(mac, ip));
		}
		qDebug() << "yourIp";
		emit hostAdded(newHost);
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
		Host* newHost;
		GMac mac = dhcpHdr->clientMac();

		QMutexLocker locker(&hosts_.m_);
		if (hostName == "") {
			Host host(mac, ip);
			newHost = hosts_.find(mac);
			if (newHost == nullptr)
				newHost = hosts_.add(host);
		} else {
			Host host(mac, ip, hostName);
			newHost = hosts_.find(mac);
			if (newHost == nullptr)
				newHost = hosts_.add(host);
			else
				newHost->name_ = host.name_;
		}
		qDebug() << "emit RequestedIpAddress";
		emit hostAdded(newHost);
	}
	return false;
}

void LiveHostMgr::captured(GPacket* packet) {
	GEthPacket* ethPacket = PEthPacket(packet);

	GEthHdr* ethHdr = ethPacket->ethHdr_;
	GMac smac = ethHdr->smac();
	if (smac == myMac_) return;

	if (processDhcp(ethPacket)) return;

	Host* newHost = nullptr;
	switch (ethHdr->type()) {
		case GEthHdr::Arp: {
			GArpHdr* arpHdr = ethPacket->arpHdr_;
			GIp sip = arpHdr->sip();
			{
				QMutexLocker locker(&hosts_.m_);
				Host* host = hosts_.find(smac);
				if (host == nullptr) {
					Host host(smac, sip);
					newHost = hosts_.add(host);
					qDebug() << "emit Arp";
				}
			}
			break;
		}
		case GEthHdr::Ip4: {
			GIpHdr* ipHdr = ethPacket->ipHdr_;
			GIp sip = myIntf_->getAdjIp(ipHdr->sip());
			if (sip == 0) return;
			{
				QMutexLocker locker(&hosts_.m_);
				Host* host = hosts_.find(smac);
				if (host == nullptr) {
					Host host(smac, sip);
					newHost = hosts_.add(host);
					qDebug() << "emit Ip";
				}
			}
			break;
		}
		default:
			break;
	}
	if (newHost != nullptr)
		emit hostAdded(newHost);
}

void LiveHostMgr::propLoad(QJsonObject jo) {
	device_.propLoad(jo);
}

void LiveHostMgr::propSave(QJsonObject& jo) {
	device_.propSave(jo);
}
