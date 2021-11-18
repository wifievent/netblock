#include "livehostmgr.h"

LiveHostMgr::LiveHostMgr(QObject* parent, GPcapDevice* pcapDevice) : GStateObj(parent), device_(pcapDevice) {
	QObject::connect(device_, &GCapture::captured, this, &LiveHostMgr::captured, Qt::DirectConnection);
}

LiveHostMgr::~LiveHostMgr() {
	close();
}

bool LiveHostMgr::doOpen() {
	hosts_.clear();

	intf_ = device_->intf();
	Q_ASSERT(intf_ != nullptr);
	myMac_ = intf_->mac();
	qDebug() << "myMac =" << QString(myMac_);

	fs_.device_ = device_;
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
	return true;
}

bool LiveHostMgr::processDhcp(GPacket* packet, GMac* mac, GIp* ip, QString* hostName) {
	GUdpHdr* udpHdr = packet->udpHdr_;
	if (udpHdr == nullptr) return false;

	if (!(udpHdr->sport() == 67 || udpHdr->dport() == 67)) return false;

	GBuf dhcp = packet->udpData_;
	if (dhcp.data_ == nullptr) return false;
	if (dhcp.size_ < sizeof(GDhcpHdr)) return false;
	GDhcpHdr* dhcpHdr = PDhcpHdr(dhcp.data_);

	bool ok = false;
	if (dhcpHdr->yourIp() != 0) { // DHCP Offer of DHCP ACK sent from server
		*mac = dhcpHdr->clientMac();
		*ip = dhcpHdr->yourIp();
		ok = true;
	}

	GEthHdr* ethHdr = packet->ethHdr_;
	if (ethHdr == nullptr) return false;
	gbyte* end = packet->buf_.data_ + packet->buf_.size_;
	GDhcpHdr::Option* option = dhcpHdr->first();
	while (true) {
		if (option->type_ == GDhcpHdr::RequestedIpAddress) {
			*ip = ntohl(*PIp(option->value()));
			*mac = ethHdr->smac();
			ok = true;
		} else if (option->type_ == GDhcpHdr::HostName) {
			for (int i = 0; i < option->len_; i++)
				*hostName += *(pchar(option->value()) + i);
		}
		option = option->next();
		if (option == nullptr) break;
		if (pbyte(option) >= end) break;
	}
	return ok;
}

bool LiveHostMgr::processArp(GEthHdr* ethHdr, GArpHdr* arpHdr, GMac* mac, GIp* ip) {
	if (ethHdr->smac() != arpHdr->smac()) {
		qDebug() << QString("ARP spoofing detected %1 %2 %3").arg(
			QString(ethHdr->smac()),
			QString(arpHdr->smac()),
			QString(arpHdr->sip()));
		return false;
	}

	*mac = arpHdr->smac();
	*ip = arpHdr->sip();
	return true;
}

bool LiveHostMgr::processIp(GEthHdr* ethHdr, GIpHdr* ipHdr, GMac* mac, GIp* ip) {
	GIp sip = ipHdr->sip();
	if (!intf_->isSameLanIp(sip)) return false;

	*mac = ethHdr->smac();
	*ip = sip;
	return true;
}

void LiveHostMgr::captured(GPacket* packet) {
	GMac mac;
	GIp ip;
	QString hostName;

	GEthHdr* ethHdr = packet->ethHdr_;
	if (ethHdr == nullptr) return;

	mac = ethHdr->smac();
	if (mac == myMac_) return;


	bool detected = false;
	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr != nullptr) {
		if (processDhcp(packet, &mac, &ip, &hostName))
			detected = true;
		else if (processIp(ethHdr, ipHdr, &mac, &ip))
			detected = true;
	}

	GArpHdr* arpHdr = packet->arpHdr_;
	if (arpHdr != nullptr) {
		if (processArp(ethHdr, arpHdr, &mac, &ip))
			detected = true;
	}

	if (!detected) return;

	if (hostName != "") {
		Host host(mac, ip, hostName);
		emit hostDetected(&host);
	}

	HostMap::iterator newHost = hosts_.end();
	{
		QMutexLocker ml(&hosts_.m_);
		HostMap::iterator it = hosts_.find(mac);
		if (it == hosts_.end()) {
			Host host(mac, ip, hostName);
			host.lastAccess_ = et_.elapsed();
			newHost = hosts_.insert(mac, host);
		} else {
			it.value().lastAccess_ = et_.elapsed();
		}
	}
	if (newHost != hosts_.end())
		emit hostDetected(&newHost.value());
}

void LiveHostMgr::propLoad(QJsonObject jo) {
    GStateObj::propLoad(jo);
    jo["FullScan"] >> fs_;
    jo["OldHostMgr"] >> ohm_;
}

void LiveHostMgr::propSave(QJsonObject& jo) {
    GStateObj::propSave(jo);
    jo["FullScan"] << fs_;
    jo["OldHostMgr"] << ohm_;
}
