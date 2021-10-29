#include "netblock.h"

NetBlock::NetBlock(QObject* parent) : GStateObj(parent) {
    QObject::connect(&device_, &GCapture::captured, this, &NetBlock::captured, Qt::DirectConnection);
}

NetBlock::~NetBlock() {
    close();
}

void NetBlock::dbCheck() {
    QSqlQuery result;
    result = nbDB_.exec("SELECT name FROM sqlite_master WHERE name = 'host'");
    qDebug() << QString("result size: %1").arg(result.size());
    result.next();
    QSqlRecord record = result.record();
    qDebug() << QString("record count: %1").arg(record.count());
    if(record.count() < 1) {
        nbDB_.exec("CREATE TABLE host (host_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, mac CHAR(17) NOT NULL, last_ip VARCHAR(15) NULL, name VARCHAR(30) NULL)");
    }
    result = nbDB_.exec("SELECT name FROM sqlite_master WHERE name = 'host'");
    qDebug() << QString("result size: %1").arg(result.size());
}

bool NetBlock::doOpen() {
    if (!lhm_.open()) {
		qWarning() << QString("lhm.open() return false %1").arg(lhm_.err->msg());
		return false;
	}

    if (!device_.open()) {
		SET_ERR(GErr::FAIL, "device_.open() return false");
		return false;
	}

    intf_ = device_.intf();
    Q_ASSERT(intf_ != nullptr);
    myMac_ = intf_->mac();
    myIp_ = intf_->ip();

    ouiDB_ = QSqlDatabase::addDatabase("QSQLITE");
    ouiDB_.setDatabaseName("oui.db");
    if(!ouiDB_.open()) {
        qWarning() << QString("ouiDB.open() return false %1").arg(ouiDB_.lastError().text());
        ouiDB_.close();
        return false;
    }

    nbDB_ = QSqlDatabase::addDatabase("QSQLITE");
    nbDB_.setDatabaseName("netblock.db");
    if(!nbDB_.open()) {
        qWarning() << QString("nbDB.open() return false %1").arg(nbDB_.lastError().text());
        nbDB_.close();
        return false;
    }

    qDebug() << QString("open success");

    thread_.start();

    return true;
}

bool NetBlock::doClose() {
    lhm_.close();
    device_.close();

    we_.wakeAll();

    ouiDB_.close();
    nbDB_.close();

    return true;
}

void NetBlock::captured(GPacket* packet) {
    GEthPacket* ethPacket = PEthPacket(packet);

    GEthHdr* ethHdr = ethPacket->ethHdr_;
    GMac smac = ethHdr->smac();
    if(smac == myMac_) return;
    if(ethHdr->type() == GEthHdr::Arp) {
        qDebug() << "Captured Arp Packet";
        GArpHdr* arpHdr = ethPacket->arpHdr_;
        if(gatewayMac_.isNull() && arpHdr->sip() == intf_->gateway()) {
            gatewayMac_ = smac;
            qDebug() << QString("gateway mac: %1").arg(QString(gatewayMac_));
        }
        if(arpHdr->op() == GArpHdr::Request) {
            HostMap::iterator iter;
            if((iter = nbHosts_.find(smac)) != nbHosts_.end() && arpHdr->tip() == intf_->gateway()) {
                qDebug() << QString("Host IP: %1").arg(QString(iter->ip_));
                sendInfect(*iter);
            }

            for(Host& host: nbHosts_) {
                if(arpHdr->tip() == host.ip_ && smac == gatewayMac_) {
                    qDebug() << QString("Host IP: %1").arg(QString(host.ip_));
                    sendInfect(host);
                    break;
                }
            }
        }
    }
}

void NetBlock::sendFindGatewayPacket() {
    EthArpPacket packet;

    GEthHdr* ethHdr = &packet.ethHdr_;
    ethHdr->dmac_ = GMac::broadcastMac();
    ethHdr->smac_ = myMac_;
    ethHdr->type_ = htons(GEthHdr::Arp);

    GArpHdr* arpHdr = &packet.arpHdr_;
    arpHdr->hrd_ = htons(GArpHdr::ETHER);
    arpHdr->pro_ = htons(GEthHdr::Ip4);
    arpHdr->hln_ = GMac::SIZE;
    arpHdr->pln_ = GIp::SIZE;
    arpHdr->op_ = htons(GArpHdr::Request);
    arpHdr->smac_ = myMac_;
    arpHdr->sip_ = myIp_;
    arpHdr->tmac_ = GMac::nullMac();
    arpHdr->tip_ = intf_->gateway();

    GPacket::Result gatewayRes = device_.write(GBuf(pbyte(&packet), sizeof(packet)));
    if (gatewayRes != GPacket::Ok) {
        qWarning() << QString("device_->write return %1 %2").arg(int(gatewayRes)).arg(device_.err->msg());
    }
}

void NetBlock::run() {
    sendFindGatewayPacket();

    while (active()) {
        block();
        if (!active()) break;
        if (we_.wait(nbUpdateTime_)) break;
    }
}

void NetBlock::block() {
    updateHosts();

    for(HostMap::iterator iter = nbHosts_.begin(); iter != nbHosts_.end(); ++iter) {
        QMutexLocker ml(&lhm_.hosts_.m_);
        if(nbNewHosts_.find(iter.key()) == nbNewHosts_.end() && lhm_.hosts_.find(iter.key()) != lhm_.hosts_.end()) {
            sendRecover(iter.value());
            if (we_.wait(sendSleepTime_)) return;
        }
    }

    {
        QMutexLocker ml(&nbHosts_.m_);
        nbHosts_.swap(nbNewHosts_);
        nbNewHosts_.clear();
    }


    for(Host& host: nbHosts_) {
        {
            QMutexLocker ml(&lhm_.hosts_.m_);
            if(lhm_.hosts_.find(host.mac_) != lhm_.hosts_.end()) {
                sendInfect(host);
            }
        }
        if (we_.wait(sendSleepTime_)) return;
    }
}

void NetBlock::updateHosts() {
    /*updateDB*/
    /*dummy data*/
    Host dum1(GMac("82:b0:17:f9:a0:75"), GIp("192.168.25.35"));
    nbNewHosts_.insert(dum1.mac_, dum1);
    Host dum2(GMac("ee:59:80:0d:c6:9c"), GIp("192.168.25.14"));
    nbNewHosts_.insert(dum1.mac_, dum2);
}

void NetBlock::sendInfect(Host host) {
    qDebug() << QString("Send Inpect %1").arg(QString(host.ip_));
    EthArpPacket packet;

    GEthHdr* ethHdr = &packet.ethHdr_;
    ethHdr->dmac_ = GMac::nullMac();
    ethHdr->smac_ = myMac_;
    ethHdr->type_ = htons(GEthHdr::Arp);

    GArpHdr* arpHdr = &packet.arpHdr_;
    arpHdr->hrd_ = htons(GArpHdr::ETHER);
    arpHdr->pro_ = htons(GEthHdr::Ip4);
    arpHdr->hln_ = GMac::SIZE;
    arpHdr->pln_ = GIp::SIZE;
    arpHdr->op_ = htons(GArpHdr::Reply);
    arpHdr->smac_ = myMac_;
    arpHdr->tmac_ = GMac::nullMac();

    //  gateway send
    ethHdr->dmac_ = gatewayMac_;
    arpHdr->sip_ = htonl(host.ip_);
    arpHdr->tmac_ = gatewayMac_;
    arpHdr->tip_ = htonl(intf_->gateway());
    GPacket::Result gatewayRes = device_.write(GBuf(pbyte(&packet), sizeof(packet)));
    if (gatewayRes != GPacket::Ok) {
        qWarning() << QString("device_->write return %1 %2").arg(int(gatewayRes)).arg(device_.err->msg());
    }

    //  target send
    ethHdr->dmac_ = host.mac_;
    arpHdr->sip_ = htonl(intf_->gateway());
    arpHdr->tmac_ = host.mac_;
    arpHdr->tip_ = htonl(host.ip_);
    GPacket::Result hostRes = device_.write(GBuf(pbyte(&packet), sizeof(packet)));
    if (hostRes != GPacket::Ok) {
        qWarning() << QString("device_->write return %1 %2").arg(int(hostRes)).arg(device_.err->msg());
    }
}

void NetBlock::sendRecover(Host host) {
    EthArpPacket packet;

    GEthHdr* ethHdr = &packet.ethHdr_;
    ethHdr->dmac_ = GMac::nullMac();
    ethHdr->smac_ = myMac_;
    ethHdr->type_ = htons(GEthHdr::Arp);

    GArpHdr* arpHdr = &packet.arpHdr_;
    arpHdr->hrd_ = htons(GArpHdr::ETHER);
    arpHdr->pro_ = htons(GEthHdr::Ip4);
    arpHdr->hln_ = GMac::SIZE;
    arpHdr->pln_ = GIp::SIZE;
    arpHdr->op_ = htons(GArpHdr::Reply);
    arpHdr->smac_ = GMac::nullMac();
    arpHdr->tmac_ = GMac::nullMac();

    //  gateway send
    ethHdr->dmac_ = gatewayMac_;
    arpHdr->smac_ = host.mac_;
    arpHdr->sip_ = htonl(host.ip_);
    arpHdr->tmac_ = gatewayMac_;
    arpHdr->tip_ = htonl(intf_->gateway());
    GPacket::Result gatewayRes = device_.write(GBuf(pbyte(&packet), sizeof(packet)));
    if (gatewayRes != GPacket::Ok) {
        qWarning() << QString("device_->write return %1 %2").arg(int(gatewayRes)).arg(device_.err->msg());
    }

    //  target send
    ethHdr->dmac_ = host.mac_;
    arpHdr->smac_ = gatewayMac_;
    arpHdr->sip_ = htonl(intf_->gateway());
    arpHdr->tmac_ = host.mac_;
    arpHdr->tip_ = htonl(host.ip_);
    GPacket::Result hostRes = device_.write(GBuf(pbyte(&packet), sizeof(packet)));
    if (hostRes != GPacket::Ok) {
        qWarning() << QString("device_->write return %1 %2").arg(int(hostRes)).arg(device_.err->msg());
    }
}

void NetBlock::propLoad(QJsonObject jo) {
    GStateObj::propLoad(jo);
    jo["NBPcapDevice"] >> device_;
    jo["LiveHostMgr"] >> lhm_;
}

void NetBlock::propSave(QJsonObject& jo) {
    GStateObj::propSave(jo);
    jo["NBPcapDevice"] << device_;
    jo["LiveHostMgr"] << lhm_;
}
