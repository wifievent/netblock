#include "netblock.h"
#include <GAtm>

NetBlock::NetBlock(QObject* parent) : GStateObj(parent) {
    QObject::connect(&device_, &GCapture::captured, this, &NetBlock::captured, Qt::DirectConnection);
}

NetBlock::~NetBlock() {
    close();
}

bool NetBlock::dbCheck() {
    qDebug() << QString("Start dbCheck Function");

    QSqlQuery nbQuery(nbDB_);
    QSqlQuery ouiQuery(ouiDB_);
    QString result;

    {
//        QMutexLocker ml(&nbDB_.m_);
        nbQuery.exec("SELECT name FROM sqlite_master WHERE name = 'host'");
    }
    nbQuery.next();
    result = nbQuery.value(0).toString();
    if(result.compare("host")) {
        qDebug() << QString("Create host table");
//        QMutexLocker ml(&nbDB_.m_);
        nbQuery.exec("CREATE TABLE host (host_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, mac CHAR(17) NOT NULL, last_ip VARCHAR(15) NULL, host_name VARCHAR(30) NULL, nick_name VARCHAR(30) NULL, oui VARCHAR(20) NULL)");
    }

    {
//        QMutexLocker ml(&nbDB_.m_);
        nbQuery.exec("SELECT name FROM sqlite_master WHERE name = 'policy'");
    }
    nbQuery.next();
    result = nbQuery.value(0).toString();
    if(result.compare("policy")) {
        qDebug() << QString("Create policy table");
//        QMutexLocker ml(&nbDB_.m_);
        nbQuery.exec("CREATE TABLE policy (policy_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, host_id INTEGER NOT NULL, start_time CHAR(4) NOT NULL, end_time CHAR(4) NOT NULL, day_of_the_week TINYINT NOT NULL)");
    }

    {
//        QMutexLocker ml(&nbDB_.m_);
        nbQuery.exec("SELECT name FROM sqlite_master WHERE name = 'block_host'");
    }
    nbQuery.next();
    result = nbQuery.value(0).toString();
    if(result.compare("block_host")) {
        qDebug() << QString("Create block_host view");
//        QMutexLocker ml(&nbDB_.m_);
        nbQuery.exec("CREATE VIEW block_host as SELECT mac, last_ip, host_name FROM host WHERE host_id in (SELECT host_id from policy where strftime(\"%H%M\", 'now', 'localtime') BETWEEN start_time AND end_time AND strftime(\"%w\", 'now', 'localtime') = day_of_the_week)");
    }

    {
//        QMutexLocker ml(&ouiDB_.m_);
        ouiQuery.exec("SELECT name FROM sqlite_master WHERE name = 'oui'");
    }
    ouiQuery.next();
    result = ouiQuery.value(0).toString();
    if(result.compare("oui")) {
        qDebug() << QString("Create oui table");
//        QMutexLocker ml(&ouiDBLock_.m_);
        ouiQuery.exec("CREATE TABLE oui (mac CHAR(20) NOT NULL PRIMARY KEY, organ VARCHAR(64) NOT NULL)");

        QFile file("manuf_rep.txt");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << QString("Not open oui file");
           // error message here
           return false;
        }

        QString getLine;
        QTextStream fileStream(&file);
        while (!fileStream.atEnd()) {
            getLine = fileStream.readLine();
            QStringList ouiResult = getLine.split('\t');
            {
//                QMutexLocker ml(&ouiDB_.m_);
                ouiQuery.prepare("INSERT INTO oui VALUES(:mac, :organ)");
                ouiQuery.bindValue(":mac", ouiResult.at(0));
                ouiQuery.bindValue(":organ", ouiResult.at(1));
                ouiQuery.exec();
            }
        }
    }

    return true;
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
    GAtm atm;
    atm.intfName_ = device_.intfName_;
    atm.insert(intf_->gateway(), GMac::nullMac());
    if(!atm.open()) {
        SET_ERR(GErr::FAIL, "atm.open() return false");
        return false;
    }
    if(!atm.wait()) {
        SET_ERR(GErr::FAIL, "atm.wait() return false");
        return false;
    }
    gatewayMac_ = atm.find(intf_->gateway()).value();

    qDebug() << "Gateway:" << QString(intf_->gateway()) << QString(gatewayMac_);

    ouiDB_ = QSqlDatabase::addDatabase("QSQLITE", "oui.db");
    ouiDB_.setDatabaseName("oui.db");
    if(!ouiDB_.open()) {
        qWarning() << QString("ouiDB.open() return false %1").arg(ouiDB_.lastError().text());
        ouiDB_.close();
        return false;
    }
    nbDB_ = QSqlDatabase::addDatabase("QSQLITE", "netblock.db");
    nbDB_.setDatabaseName("netblock.db");
    if(!nbDB_.open()) {
        qWarning() << QString("nbDB.open() return false %1").arg(nbDB_.lastError().text());
        nbDB_.close();
        return false;
    }

    if(!dbCheck()) {
        return false;
    }

    thread_.start();

    qDebug() << QString("open success");

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
    if(smac == myMac_) {
#ifdef Q_OS_WIN
        if(ethHdr->dmac() == myMac_) {
            packet->ethHdr_->dmac_ = gatewayMac_;
            device_.write(packet);
        }
#endif
        return;
    }
    if(ethHdr->type() == GEthHdr::Arp) {
        qDebug() << "Captured Arp Packet";
        GArpHdr* arpHdr = ethPacket->arpHdr_;
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

void NetBlock::run() {
    qDebug() << "NeBlock RUN!!!!!!!!";


    while (active()) {
        updateHosts();
        block();
        if (!active()) break;
        if (we_.wait(nbUpdateTime_)) break;
    }
}

void NetBlock::block() {
    qDebug() << "in block function";

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
    qDebug() << "Update NB Hosts";
    /*updateDB*/
    QSqlQuery nbQuery(nbDB_);
    nbQuery.exec("SELECT * FROM block_host");
    while(nbQuery.next()) {
        Host host(GMac(nbQuery.value(0).toString()), GIp(nbQuery.value(1).toString()), nbQuery.value(2).toString());
        nbNewHosts_.insert(host.mac_, host);
    }
}

void NetBlock::sendInfect(Host host) {
    qDebug() << QString("Send Infect %1").arg(QString(host.ip_));
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
