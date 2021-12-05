#include "oldhostmgr.h"
#include "livehostmgr.h"

void stdActiveScanThread::run() {
    // qDebug() << "beg";
    PcapDevice* device = ohm_->lhm_->device_;
    Intf* intf = device->intf();
    Q_ASSERT(intf != nullptr);

    Ip myIp = intf->ip();
    Mac myMac = intf->mac();

    EthArpPacket packet;

    EthHdr* ethHdr = &packet.ethHdr_;
    ethHdr->dmac_ = host_->mac_; // Unicast
    ethHdr->smac_ = myMac;
    ethHdr->type_ = htons(GEthHdr::Arp);

    ArpHdr* arpHdr = &packet.arpHdr_;
    arpHdr->hrd_ = htons(ArpHdr::ETHER);
    arpHdr->pro_ = htons(EthHdr::Ip4);
    arpHdr->hln_ = Mac::SIZE;
    arpHdr->pln_ = Ip::SIZE;
    arpHdr->op_ = htons(ArpHdr::Request);
    arpHdr->smac_ = myMac;
    arpHdr->sip_ = htonl(myIp);
    arpHdr->tmac_ = Mac::nullMac();
    arpHdr->tip_ = htonl(host_->ip_);

    QElapsedTimer et;
    qint64 start = et.elapsed();

    auto stdstart = std::chrono::steady_clock::now();
    while (true) {
        Packet::Result res = device->write(Buf(pbyte(&packet), sizeof(packet)));
        if (res != Packet::Ok) {
            qWarning() << QString("device_->write return %1").arg(int(res));
        }

        std::unique_lock<std::mutex> lock(myMutex_);
        if(myCv_.wait_for(lock,std::chrono::milliseconds(ohm_->sendSleepTime_)) == std::cv_status::no_timeout) break;

        qint64 now = et.elapsed();

        auto stdnow = std::chrono::steady_clock::now();

        if (host_->lastAccess_ + ohm_->scanStartTimeout_ > now) { // accessed
            qDebug() << QString("detect %1 %2").arg(QString(std::string(host_->mac_).data()), QString(std::string(host_->ip_).data()));
            break;
        }

        // qDebug() << QString("start=%1 now=%2 diff=%3").arg(start).arg(now).arg(now - start); // gilgil temp 2021.10.24
        if (start + ohm_->deleteTimeout_ < now) {
            qDebug() << QString("start=%1 now=%2 diff=%3").arg(start).arg(now).arg(now - start);
            emit ohm_->lhm_->hostDeleted(host_);
            QMutexLocker ml(&ohm_->lhm_->hosts_.m_);
            ohm_->lhm_->hosts_.remove(host_->mac_);
            break;
        }
    }

    stdActiveScanThreadMap* sastm = &ohm_->sastm_;
    QMutexLocker ml(&sastm->m_);
    int res = sastm->erase(host_->mac_);
    if (res != 1) {
        qDebug() << QString("astm->remove return %1").arg(res);
    }
    // qDebug() << "end";
}

ActiveScanThread::ActiveScanThread(OldHostMgr* ohm, Host* host) : GThread(nullptr) {
	ohm_ = ohm;
	host_ = host;
	QObject::connect(this, &QThread::finished, this, &QThread::deleteLater);
}

ActiveScanThread::~ActiveScanThread() {
}

void ActiveScanThread::run() {
	// qDebug() << "beg";
    PcapDevice* device = ohm_->lhm_->device_;
    Intf* intf = device->intf();
	Q_ASSERT(intf != nullptr);

    Ip myIp = intf->ip();
    Mac myMac = intf->mac();

	EthArpPacket packet;

    EthHdr* ethHdr = &packet.ethHdr_;
	ethHdr->dmac_ = host_->mac_; // Unicast
	ethHdr->smac_ = myMac;
	ethHdr->type_ = htons(GEthHdr::Arp);

    ArpHdr* arpHdr = &packet.arpHdr_;
    arpHdr->hrd_ = htons(ArpHdr::ETHER);
    arpHdr->pro_ = htons(EthHdr::Ip4);
    arpHdr->hln_ = Mac::SIZE;
    arpHdr->pln_ = Ip::SIZE;
    arpHdr->op_ = htons(ArpHdr::Request);
	arpHdr->smac_ = myMac;
	arpHdr->sip_ = htonl(myIp);
    arpHdr->tmac_ = Mac::nullMac();
	arpHdr->tip_ = htonl(host_->ip_);

	QElapsedTimer et;
	qint64 start = et.elapsed();
	while (true) {
        Packet::Result res = device->write(Buf(pbyte(&packet), sizeof(packet)));
        if (res != Packet::Ok) {
            qWarning() << QString("device_->write return %1").arg(int(res));
		}
		if (we_.wait(ohm_->sendSleepTime_)) break;

		qint64 now = et.elapsed();
		if (host_->lastAccess_ + ohm_->scanStartTimeout_ > now) { // accessed
            qDebug() << QString("detect %1 %2").arg(QString(std::string(host_->mac_).data()), QString(std::string(host_->ip_).data()));
			break;
		}

		// qDebug() << QString("start=%1 now=%2 diff=%3").arg(start).arg(now).arg(now - start); // gilgil temp 2021.10.24
		if (start + ohm_->deleteTimeout_ < now) {
			qDebug() << QString("start=%1 now=%2 diff=%3").arg(start).arg(now).arg(now - start);
			emit ohm_->lhm_->hostDeleted(host_);
			QMutexLocker ml(&ohm_->lhm_->hosts_.m_);
			ohm_->lhm_->hosts_.remove(host_->mac_);
			break;
		}
	}

	ActiveScanThreadMap* astm = &ohm_->astm_;
	QMutexLocker ml(&astm->m_);
	int res = astm->remove(host_->mac_);
	if (res != 1) {
		qDebug() << QString("astm->remove return %1").arg(res);
	}
	// qDebug() << "end";
}


OldHostMgr::OldHostMgr(QObject* parent) : GStateObj(parent) {
}

OldHostMgr::~OldHostMgr() {
	close();
}

bool OldHostMgr::doOpen() {
	if (lhm_ == nullptr) {
		SET_ERR(GErr::OBJECT_IS_NULL, "lhm_ is null");
		return false;
    }

    myThread_ = new std::thread(&OldHostMgr::run, this);

	return true;
}

bool OldHostMgr::doClose() {
    myCv_.notify_all();
    myThread_->join();

	return true;
}

void OldHostMgr::run() {
	qDebug() << "beg";

	QElapsedTimer et;
	while (active()) {
		{
			QMutexLocker ml(&lhm_->hosts_.m_);
			qint64 now = et.elapsed();
			for (Host& host : lhm_->hosts_) {
				// qDebug() << QString("lastAccess=%1 now=%2 diff=%3").arg(host.lastAccess_).arg(now).arg(now-host.lastAccess_); // gilgil temp 2021.10.24
				if (host.lastAccess_ + scanStartTimeout_ < now) {
                    QMutexLocker ml(&sastm_.m_);
                    stdActiveScanThreadMap::iterator it = sastm_.find(host.mac_);
                    if (it == sastm_.end()) {
                        stdActiveScanThread* sast = new stdActiveScanThread(this, &host);
                        sastm_.insert(std::pair<Mac, stdActiveScanThread*>(host.mac_, sast));
					}
				}
			}
        }

        std::unique_lock<std::mutex> lock(myMutex_);
        if(myCv_.wait_for(lock,std::chrono::milliseconds(sendSleepTime_)) == std::cv_status::no_timeout) break;
	}

	qDebug() << "end";
}

OldHostMgr::MyThread::MyThread(QObject *parent) : GThread(parent) {
}

OldHostMgr::MyThread::~MyThread() {
}

void OldHostMgr::MyThread::run() {
    OldHostMgr* ohm = dynamic_cast<OldHostMgr*>(parent());
    Q_ASSERT(ohm != nullptr);
    ohm->run();
}
