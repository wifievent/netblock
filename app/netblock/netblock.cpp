#include "netblock.h"
#include <GAtm>

#include <glog/logging.h>

bool NetBlock::dbCheck()
{
    DLOG(INFO) << "Start dbCheck Function";

    QSqlQuery nbQuery(nbDB_);

    std::list<DataList> result;
    result = nbConnect_->selectQuery("SELECT name FROM sqlite_master WHERE name = 'host'");
    if(result.size() == 0)
    {
        DLOG(INFO) << "Create host table";
        nbConnect_->sendQuery("CREATE TABLE host (host_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, mac CHAR(17) NOT NULL, last_ip VARCHAR(15) NULL, host_name VARCHAR(30) NULL, nick_name VARCHAR(30) NULL, oui VARCHAR(20) NULL)");
    }
    result.clear();

    result = nbConnect_->selectQuery("SELECT name FROM sqlite_master WHERE name = 'policy'");
    if(result.size() == 0)
    {
        DLOG(INFO) << "Create policy table";
        nbConnect_->sendQuery("CREATE TABLE policy (policy_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, host_id INTEGER NOT NULL, start_time CHAR(4) NOT NULL, end_time CHAR(4) NOT NULL, day_of_the_week TINYINT NOT NULL)");
    }
    result.clear();

    result = nbConnect_->selectQuery("SELECT name FROM sqlite_master WHERE name = 'block_host'");
    if(result.size() == 0)
    {
        DLOG(INFO) << "Create block_host view";
        nbConnect_->sendQuery("CREATE VIEW block_host as SELECT mac, last_ip, host_name FROM host WHERE host_id in (SELECT host_id from policy where strftime(\"%H%M\", 'now', 'localtime') BETWEEN start_time AND end_time AND strftime(\"%w\", 'now', 'localtime') = day_of_the_week)");
    }
    result.clear();

    return true;
}

void NetBlock::findGatewayMac()
{
    DLOG(INFO) << "Find gateway mac";
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
	if (!device_.open())
    {
        return false;
    }

	if (!lhm_.open())
	{
        DLOG(WARNING) << "lhm.opoen() return false";
		return false;
	}

    intf_ = device_.intf();
    myMac_ = intf_->mac();
    myIp_ = intf_->ip();

    findGatewayMac();
    if(gatewayMac_ == Mac::nullMac()) {
        return false;
    }

    DLOG(INFO) << "Gateway: " << std::string(intf_->gateway()).data() << std::string(gatewayMac_).data();

    ouiConnect_ = new DBConnect(std::string("oui.db"));
    if(!ouiConnect_->open())
    {
        DLOG(WARNING) << "ouiConnect return false";
        return false;
    }
    ouiDB_ = QSqlDatabase::addDatabase("QSQLITE", "oui.db");
    ouiDB_.setDatabaseName("oui.db");
    if (!ouiDB_.open())
    {
        qWarning() << QString("ouiDB.open() return false %1").arg(ouiDB_.lastError().text());
        ouiDB_.close();
        return false;
    }

    nbConnect_ = new DBConnect(std::string("netblock.db"));
    if(!nbConnect_->open())
    {
        DLOG(WARNING) << "nbConnect return false";
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
    for(StdHostMap::iterator iter = nbHosts_.begin(); iter != nbHosts_.end(); ++iter)
    {
        sendRecover(iter->second);
    }

    lhm_.close();
    device_.close();

    dbCv_.notify_all();
    infectCv_.notify_all();
    dbUpdateThread_->join();
    infectThread_->join();

    ouiDB_.close();
    nbDB_.close();


    ouiConnect_->close();
    nbConnect_->close();

    delete ouiConnect_;
    delete nbConnect_;

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
        StdHostMap::iterator iter;
        if ((iter = nbHosts_.find(smac)) != nbHosts_.end() && arpHdr->tip() == intf_->gateway())
        {
            DLOG(INFO) << "Host Ip:" << std::string(iter->second.ip_).data();
            sendInfect(iter->second);
        }

        for (StdHostMap::iterator iter = nbHosts_.begin(); iter != nbHosts_.end(); ++iter)
        {
            if (arpHdr->tip() == iter->second.ip_ && smac == gatewayMac_)
            {
                DLOG(INFO) << "Host IP:" << std::string(iter->second.ip_).data();
                sendInfect(iter->second);
                break;
            }
        }
    }
}

void NetBlock::block()
{
	//qDebug() << "is block function";
    {
        std::lock_guard<std::mutex> lock(nbHosts_.m_);
        for(StdHostMap::iterator iter = nbHosts_.begin(); iter != nbHosts_.end(); ++iter)
        {
            {
                std::lock_guard<std::mutex> lock(lhm_.hosts_.m_);
                if(lhm_.hosts_.find(iter->second.mac_) != lhm_.hosts_.end())
                {
                    sendInfect(iter->second);
                }
            }
        }
        for(const std::pair<Mac, StdHost> host : nbHosts_)
        {
            {
                std::lock_guard<std::mutex> lock(lhm_.hosts_.m_);
                if(lhm_.hosts_.find(host.second.mac_) != lhm_.hosts_.end())
                {
                    sendInfect(host.second);
                }
            }

            std::unique_lock<std::mutex> lock(blockMutex_);
            if(blockCv_.wait_for(lock,std::chrono::milliseconds(sendSleepTime_)) == std::cv_status::no_timeout) break;
        }
    }
}

void NetBlock::updateHosts()
{
    DLOG(INFO) << "Update NB Hosts";
    /*updateDB*/
    std::list<DataList> dl = nbConnect_->selectQuery(std::string("SELECT * FROM block_host"));

    for(DataList dataList : dl)
    {
        StdHost host(Mac(dataList.argv_[0]), Ip(dataList.argv_[1]), dataList.argv_[2]);
        nbNewHosts_.insert(std::pair<Mac, StdHost>(host.mac_, host));
    }

    for (StdHostMap::iterator iter = nbHosts_.begin(); iter != nbHosts_.end(); ++iter)
    {
        std::lock_guard<std::mutex> lock(lhm_.hosts_.m_);
        if(nbNewHosts_.find(iter->first) == nbNewHosts_.end() && lhm_.hosts_.find(iter->first) != lhm_.hosts_.end())
        {
            sendRecover(iter->second);
            std::unique_lock<std::mutex> lock(blockMutex_);
            if(blockCv_.wait_for(lock,std::chrono::milliseconds(sendSleepTime_)) == std::cv_status::no_timeout) return;

        }
    }

    {
        std::lock_guard<std::mutex> lock(nbHosts_.m_);
        nbHosts_.swap(nbNewHosts_);
        nbNewHosts_.clear();
    }
}

void NetBlock::sendInfect(StdHost host)
{
    DLOG(INFO) << "Infect:" << std::string(host.ip_).data();
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
        DLOG(WARNING) << "device_->write return" << int(gatewayRes);
    }

    //  target send
    ethHdr->dmac_ = host.mac_;
    arpHdr->sip_ = htonl(intf_->gateway());
    arpHdr->tmac_ = host.mac_;
    arpHdr->tip_ = htonl(host.ip_);
    Packet::Result hostRes = device_.write(Buf(pbyte(&packet), sizeof(packet)));
    if (hostRes != Packet::Ok)
    {
        DLOG(WARNING) << "device_->write return" << int(hostRes);
    }
}

void NetBlock::sendRecover(StdHost host)
{
    DLOG(INFO) << "Recover" << std::string(host.ip_).data();
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
        DLOG(WARNING) << "device_->write return" << int(gatewayRes);
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
        DLOG(WARNING)<< "device_->write return" << int(hostRes);
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

void NetBlock::load(Json::Value &json)
{
    json["sendSleepTime"] >> sendSleepTime_;
    json["nbUpdateTime"] >> nbUpdateTime_;
    json["infectSleepTime"] >> infectSleepTime_;
    device_ << json["NBPcapDevice"];
    lhm_ << json["LiveHostMgr"];
}

void NetBlock::save(Json::Value &json)
{
    json["sendSleepTime"] << sendSleepTime_;
    json["nbUpdateTime"] << nbUpdateTime_;
    json["infectSleepTime"] << infectSleepTime_;
    device_ >> json["NBPcapDevice"];
    lhm_ >> json["LiveHostMgr"];
}
