#include "netblock.h"
#include <GAtm>

NetBlock::NetBlock(QObject *parent) : GStateObj(parent)
{
}

NetBlock::~NetBlock()
{
    close();
}

bool NetBlock::dbCheck()
{
    qDebug() << QString("Start dbCheck Function");

    QSqlQuery nbQuery(nbDB_);
    QString result;

    {
        QMutexLocker ml(&nbDBLock_);
        nbQuery.exec("SELECT name FROM sqlite_master WHERE name = 'host'");

        nbQuery.next();
        result = nbQuery.value(0).toString();
    }
    if (result.compare("host"))
    {
        qDebug() << QString("Create host table");
        QMutexLocker ml(&nbDBLock_);
        nbQuery.exec("CREATE TABLE host (host_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, mac CHAR(17) NOT NULL, last_ip VARCHAR(15) NULL, host_name VARCHAR(30) NULL, nick_name VARCHAR(30) NULL, oui VARCHAR(20) NULL)");
    }

    {
        QMutexLocker ml(&nbDBLock_);
        nbQuery.exec("SELECT name FROM sqlite_master WHERE name = 'policy'");

        nbQuery.next();
        result = nbQuery.value(0).toString();
    }
    if (result.compare("policy"))
    {
        qDebug() << QString("Create policy table");
        QMutexLocker ml(&nbDBLock_);
        nbQuery.exec("CREATE TABLE policy (policy_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, host_id INTEGER NOT NULL, start_time CHAR(4) NOT NULL, end_time CHAR(4) NOT NULL, day_of_the_week TINYINT NOT NULL)");
    }

    {
        QMutexLocker ml(&nbDBLock_);
        nbQuery.exec("SELECT name FROM sqlite_master WHERE name = 'block_host'");

        nbQuery.next();
        result = nbQuery.value(0).toString();
    }

    if (result.compare("block_host"))
    {
        qDebug() << QString("Create block_host view");
        QMutexLocker ml(&nbDBLock_);
        nbQuery.exec("CREATE VIEW block_host as SELECT mac, last_ip, host_name FROM host WHERE host_id in (SELECT host_id from policy where strftime(\"%H%M\", 'now', 'localtime') BETWEEN start_time AND end_time AND strftime(\"%w\", 'now', 'localtime') = day_of_the_week)");
    }

    return true;
}

void NetBlock::findGatewayMac()
{
    qDebug() << "Find gateway Mac";
    EthArpPacket packet;

    EthHdr *ethHdr = &packet.ethHdr_;
    ethHdr->dmac_ = Mac::broadcastMac();
    ethHdr->smac_ = myMac_;
    ethHdr->type_ = htons(EthHdr::Arp);

    ArpHdr *arpHdr = &packet.arpHdr_;
    arpHdr->hrd_ = htons(ArpHdr::ETHER);
    arpHdr->pro_ = htons(EthHdr::Ip4);
    arpHdr->hln_ = Mac::SIZE;
    arpHdr->pln_ = Ip::SIZE;
    arpHdr->op_ = htons(ArpHdr::Request);
    arpHdr->smac_ = myMac_;
    arpHdr->sip_ = myIp_;
    arpHdr->tmac_ = Mac::nullMac();
    arpHdr->tip_ = intf_->gateway_;

    Packet::Result gatewayRes = device_.write(Buf(pbyte(&packet), sizeof(packet)));
    if (gatewayRes != Packet::Ok)
    {
        qWarning() << QString("device_->write return %1").arg(int(gatewayRes));
    }

    while(true)
    {
        Packet* readPacket;
        Packet::Result res = device_.read(readPacket);
        if (res == Packet::Eof) {
            break;
        } else
        if (res == Packet::Fail) {
            break;
        } else
        if (res == Packet::None) {
            continue;
        }

        EthPacket *ethPacket = PEthPacket(readPacket);

        ArpHdr* arpHdr = readPacket->arpHdr_;

        if(arpHdr == nullptr) continue;
        if(arpHdr->op() != ArpHdr::Reply) continue;

        Ip sIp = arpHdr->sip();
        Mac sMac = arpHdr->smac();

        if(intf_->gateway() == sIp) {
            gatewayMac_ = sMac;
            break;
        }

    }
}

bool NetBlock::doOpen()
{
    qDebug() << getenv("HOME");
	if (!device_.open())
    {
        SET_ERR(GErr::FAIL, "device_.open() return false");
        return false;
    }

	if (!lhm_.open())
	{
		qWarning() << QString("lhm.open() return false %1").arg(lhm_.err->msg());
		return false;
	}

    intf_ = device_.intf();
    Q_ASSERT(intf_ != nullptr);
    myMac_ = intf_->mac();
    myIp_ = intf_->ip();

    findGatewayMac();
    if(gatewayMac_ == Mac::nullMac()) {
        return false;
    }

    qDebug() << "Gateway:" << QString(std::string(intf_->gateway()).data()) << QString(std::string(gatewayMac_).data());

    ouiDB_ = QSqlDatabase::addDatabase("QSQLITE", "oui.db");
    ouiDB_.setDatabaseName("oui.db");
    if (!ouiDB_.open())
    {
        qWarning() << QString("ouiDB.open() return false %1").arg(ouiDB_.lastError().text());
        ouiDB_.close();
        return false;
    }
    nbDB_ = QSqlDatabase::addDatabase("QSQLITE", "netblock.db");
    // Windows: FOLDERID_Profile
    //PWSTR path = NULL;
    //SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, &path); //User directory

    nbDB_.setDatabaseName("~/.config/netblock.db");
    if (!nbDB_.open())
    {
        qWarning() << QString("nbDB.open() return false %1").arg(nbDB_.lastError().text());
        nbDB_.close();
        return false;
    }

    if (!dbCheck())
    {
        return false;
    }

    dbUpdateThread_ = new std::thread(&NetBlock::dbUpdateRun, this);
    infectThread_ = new std::thread(&NetBlock::infectRun, this);

    qDebug() << QString("open success");

    return true;
}

bool NetBlock::doClose()
{
    for (Host &host : nbHosts_)
    {
        sendRecover(host);
    }

    lhm_.close();
    device_.close();

    dbCv_.notify_all();
    infectCv_.notify_all();
    dbUpdateThread_->join();
    infectThread_->join();

    ouiDB_.close();
    nbDB_.close();

    return true;
}

void NetBlock::capture() {
    Packet* packet;
    while(active())
    {
        Packet::Result res = device_.read(packet);
        if(res == Packet::None) continue;
        if(res == Packet::Eof || res == Packet::Fail) break;
        captured(packet);
        lhm_.captured(packet);
    }
}

void NetBlock::captured(Packet *packet)
{
    EthPacket *ethPacket = PEthPacket(packet);

    EthHdr *ethHdr = ethPacket->ethHdr_;
    Mac smac = ethHdr->smac();
    if (smac == myMac_)
    {
#ifdef Q_OS_WIN
        if (ethHdr->dmac() == myMac_)
        {
            packet->ethHdr_->dmac_ = gatewayMac_;
            device_.write(packet);
        }
#endif
        return;
    }
    if (ethHdr->type() == GEthHdr::Arp)
    {
		// qDebug() << "Captured Arp Packet";
        ArpHdr *arpHdr = ethPacket->arpHdr_;
        HostMap::iterator iter;
        if ((iter = nbHosts_.find(smac)) != nbHosts_.end() && arpHdr->tip() == intf_->gateway())
        {
            qDebug() << QString("Host IP: %1").arg(QString(std::string(iter->ip_).data()));
            sendInfect(*iter);
        }

        for (Host &host : nbHosts_)
        {
            if (arpHdr->tip() == host.ip_ && smac == gatewayMac_)
            {
                qDebug() << QString("Host IP: %1").arg(QString(std::string(host.ip_).data()));
                sendInfect(host);
                break;
            }
        }
    }
}

void NetBlock::block()
{
	//qDebug() << "is block function";
    {
        QMutexLocker ml(&nbHosts_.m_);
        for (Host &host : nbHosts_)
        {
            {
                QMutexLocker ml(&lhm_.hosts_.m_);
                if (lhm_.hosts_.find(host.mac_) != lhm_.hosts_.end())
                {
                    sendInfect(host);
                }
            }
            if (we_.wait(sendSleepTime_))
                break;
            ;
        }
    }
}

void NetBlock::updateHosts()
{
    qDebug() << "Update NB Hosts";
    /*updateDB*/
    QSqlQuery nbQuery(nbDB_);
    {
        QMutexLocker ml(&nbDBLock_);
        nbQuery.exec("SELECT * FROM block_host");

        while (nbQuery.next())
        {
            Host host(Mac(nbQuery.value(0).toString().toStdString()), Ip(nbQuery.value(1).toString().toStdString()), nbQuery.value(2).toString());
            nbNewHosts_.insert(host.mac_, host);
        }
    }

    for (HostMap::iterator iter = nbHosts_.begin(); iter != nbHosts_.end(); ++iter)
    {
        QMutexLocker ml(&lhm_.hosts_.m_);
        if (nbNewHosts_.find(iter.key()) == nbNewHosts_.end() && lhm_.hosts_.find(iter.key()) != lhm_.hosts_.end())
        {
            sendRecover(iter.value());
            if (we_.wait(sendSleepTime_))
                return;
        }
    }

    {
        QMutexLocker ml(&nbHosts_.m_);
        nbHosts_.swap(nbNewHosts_);
        nbNewHosts_.clear();
    }
}

void NetBlock::sendInfect(Host host)
{
    qDebug() << QString(std::string(host.ip_).data());
    EthArpPacket packet;

    EthHdr *ethHdr = &packet.ethHdr_;
    ethHdr->dmac_ = Mac::nullMac();
    ethHdr->smac_ = myMac_;
    ethHdr->type_ = htons(EthHdr::Arp);

    ArpHdr *arpHdr = &packet.arpHdr_;
    arpHdr->hrd_ = htons(ArpHdr::ETHER);
    arpHdr->pro_ = htons(EthHdr::Ip4);
    arpHdr->hln_ = Mac::SIZE;
    arpHdr->pln_ = Ip::SIZE;
    arpHdr->op_ = htons(ArpHdr::Reply);
    arpHdr->smac_ = myMac_;
    arpHdr->tmac_ = Mac::nullMac();

    //  gateway send
    ethHdr->dmac_ = gatewayMac_;
    arpHdr->sip_ = htonl(host.ip_);
    arpHdr->tmac_ = gatewayMac_;
    arpHdr->tip_ = htonl(intf_->gateway());
    Packet::Result gatewayRes = device_.write(Buf(pbyte(&packet), sizeof(packet)));
    if (gatewayRes != Packet::Ok)
    {
        qWarning() << QString("device_->write return %1").arg(int(gatewayRes));
    }

    //  target send
    ethHdr->dmac_ = host.mac_;
    arpHdr->sip_ = htonl(intf_->gateway());
    arpHdr->tmac_ = host.mac_;
    arpHdr->tip_ = htonl(host.ip_);
    Packet::Result hostRes = device_.write(Buf(pbyte(&packet), sizeof(packet)));
    if (hostRes != Packet::Ok)
    {
        qWarning() << QString("device_->write return %1").arg(int(hostRes));
    }
}

void NetBlock::sendRecover(Host host)
{
    qDebug() << QString(std::string(host.ip_).data());
    EthArpPacket packet;

    EthHdr *ethHdr = &packet.ethHdr_;
    ethHdr->dmac_ = Mac::nullMac();
    ethHdr->smac_ = myMac_;
    ethHdr->type_ = htons(GEthHdr::Arp);

    ArpHdr *arpHdr = &packet.arpHdr_;
    arpHdr->hrd_ = htons(ArpHdr::ETHER);
    arpHdr->pro_ = htons(EthHdr::Ip4);
    arpHdr->hln_ = Mac::SIZE;
    arpHdr->pln_ = Ip::SIZE;
    arpHdr->op_ = htons(ArpHdr::Reply);
    arpHdr->smac_ = Mac::nullMac();
    arpHdr->tmac_ = Mac::nullMac();

    //  gateway send
    ethHdr->dmac_ = gatewayMac_;
    arpHdr->smac_ = host.mac_;
    arpHdr->sip_ = htonl(host.ip_);
    arpHdr->tmac_ = gatewayMac_;
    arpHdr->tip_ = htonl(intf_->gateway());
    Packet::Result gatewayRes = device_.write(Buf(pbyte(&packet), sizeof(packet)));
    if (gatewayRes != Packet::Ok)
    {
        qWarning() << QString("device_->write return %1").arg(int(gatewayRes));
    }

    //  target send
    ethHdr->dmac_ = host.mac_;
    arpHdr->smac_ = gatewayMac_;
    arpHdr->sip_ = htonl(intf_->gateway());
    arpHdr->tmac_ = host.mac_;
    arpHdr->tip_ = htonl(host.ip_);
    Packet::Result hostRes = device_.write(Buf(pbyte(&packet), sizeof(packet)));
    if (hostRes != Packet::Ok)
    {
        qWarning() << QString("device_->write return %1").arg(int(hostRes));
    }
}

void NetBlock::dbUpdateRun()
{
    while(active())
    {
        updateHosts();
        if(!active())
            break;

        std::unique_lock<std::mutex> lock(dbMutex_);
        if(dbCv_.wait_for(lock,std::chrono::milliseconds(nbUpdateTime_)) == std::cv_status::no_timeout)
            break;
    }
}

void NetBlock::infectRun()
{
    while(active())
    {
        block();
        if(!active())
            break;

        std::unique_lock<std::mutex> lock(infectMutex_);
        if(infectCv_.wait_for(lock,std::chrono::milliseconds(infectSleepTime_)) == std::cv_status::no_timeout)
            break;
    }
}

void NetBlock::propLoad(QJsonObject jo)
{
    GStateObj::propLoad(jo);
    jo["NBPcapDevice"] >> device_;
    jo["LiveHostMgr"] >> lhm_;
}

void NetBlock::propSave(QJsonObject &jo)
{
    GStateObj::propSave(jo);
    jo["NBPcapDevice"] << device_;
    jo["LiveHostMgr"] << lhm_;
}
