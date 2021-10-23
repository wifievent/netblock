#include "oldhostmgr.h"
#include "livehostmgr.h"

ActiveScanThread::ActiveScanThread(QObject* owner, Host* host) : GThread(owner) {
	ohm_ = dynamic_cast<OldHostMgr*>(owner);
	host_ = host;
}

ActiveScanThread::~ActiveScanThread() {
	ActiveScanThreadMap* astm = &ohm_->astm_;
	QMutexLocker ml(&astm->m_);
	int res = astm->remove(host_->mac_);
	qDebug() << QString("astm->remove return %1").arg(res);
	deleteLater();
}

void ActiveScanThread::run() {
	qDebug() << "beg";

	GPcapDevice* device = &ohm_->lhm_->device_;
	GIntf* intf = device->intf();
	Q_ASSERT(intf != nullptr);

	GIp myIp = intf->ip();
	GMac myMac = intf->mac();

	EthArpPacket packet;

	GEthHdr* ethHdr = &packet.ethHdr_;
	ethHdr->dmac_ = host_->mac_; // Unicast
	ethHdr->smac_ = myMac;
	ethHdr->type_ = htons(GEthHdr::Arp);

	GArpHdr* arpHdr = &packet.arpHdr_;
	arpHdr->hrd_ = htons(GArpHdr::ETHER);
	arpHdr->pro_ = htons(GEthHdr::Ip4);
	arpHdr->hln_ = GMac::SIZE;
	arpHdr->pln_ = GIp::SIZE;
	arpHdr->op_ = htons(GArpHdr::Request);
	arpHdr->smac_ = myMac;
	arpHdr->sip_ = htonl(myIp);
	arpHdr->tmac_ = GMac::nullMac();

	QElapsedTimer et;
	qint64 start = et.elapsed();
	while (true) {
		GPacket::Result res = device->write(GBuf(pbyte(&packet), sizeof(packet)));
		if (res != GPacket::Ok) {
			qWarning() << QString("device_->write return %1 %2").arg(int(res)).arg(device->err->msg());
		}
		if (we_.wait(ohm_->sendSleepTime_)) break;

		qint64 now = et.elapsed();
		if (start + ohm_->scanStartTimeout_ > now) { // accessed
			qDebug() << QString("access detected %1").arg(QString(host_->mac_));
			break;
		}

		if (start + ohm_->deleteTimeout_ > now) {
			qDebug() << QString("start=%1 now=%2 diff=%3").arg(start, now, not - start);
			emit ohm_->lhm_->hostDeleted(host_);
			break;
		}
	}

	qDebug() << "end";
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

	thread_.start();

	return true;
}

bool OldHostMgr::doClose() {
	we_.wakeAll();

	if (!thread_.wait() ) {
		qCritical() << "thread_.wait return false";
	}

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
				if (host.lastAccess_ + scanStartTimeout_ < now) {
					QMutexLocker ml(&astm_.m_);
					ActiveScanThreadMap::iterator it = astm_.find(host.mac_);
					if (it == astm_.end()) {
						ActiveScanThread* ast = new ActiveScanThread(this, &host);
						astm_.insert(host.mac_, ast);
						ast->start();
					}
				}
			}
		}
		if (we_.wait(checkSleepTime_)) break;
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
