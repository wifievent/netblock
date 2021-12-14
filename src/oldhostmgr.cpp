#include "oldhostmgr.h"
#include "livehostmgr.h"

#include <sys/time.h>

#include <glog/logging.h>

void StdActiveScanThread::run() {
    PcapDevice* device = ohm_->lhm_->device_;
    Intf* intf = device->intf();
    Q_ASSERT(intf != nullptr);

    Ip myIp = intf->ip();
    Mac myMac = intf->mac();

    EthArpPacket packet;

    EthHdr* ethHdr = &packet.ethHdr_;
    ethHdr->dmac_ = host_->mac_; // Unicast
    ethHdr->smac_ = myMac;
    ethHdr->type_ = htons(EthHdr::Arp);

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

    struct timeval start;
    gettimeofday(&start, NULL);

    while (true) {
        Packet::Result res = device->write(Buf(pbyte(&packet), sizeof(packet)));
        if (res != Packet::Ok) {
            DLOG(WARNING) << "device_->write return" << int(res);
        }

        std::unique_lock<std::mutex> lock(myMutex_);
        if(myCv_.wait_for(lock,std::chrono::milliseconds(ohm_->sendSleepTime_)) == std::cv_status::no_timeout) break;

        struct timeval now;
        gettimeofday(&now, NULL);

        if (host_->lastAccess_ + ohm_->scanStartTimeout_ > now.tv_sec) { // accessed
            DLOG(INFO) << "detect" << std::string(host_->mac_).data() << std::string(host_->ip_).data();
            break;
        }

        // qDebug() << QString("start=%1 now=%2 diff=%3").arg(start).arg(now).arg(now - start); // gilgil temp 2021.10.24
        if (start.tv_sec + ohm_->deleteTimeout_ < now.tv_sec) {
            DLOG(INFO) << "start=" << start.tv_sec << "now=" << now.tv_sec << "diff=" << now.tv_sec - start.tv_sec;
            emit ohm_->lhm_->hostDeleted(host_);
            std::lock_guard<std::mutex> lock(ohm_->lhm_->hosts_.m_);
            ohm_->lhm_->hosts_.erase(host_->mac_);
            break;
        }
    }

    StdActiveScanThreadMap* sastm = &ohm_->sastm_;
    std::lock_guard<std::mutex> lock(sastm->m_);
    int res = sastm->erase(host_->mac_);
    if (res != 1) {
        DLOG(INFO) << "astm->remove return" << res;
    }
}

bool StdOldHostMgr::doOpen()
{
    if(lhm_ == nullptr)
    {
        return false;
    }

    myThread_ = new std::thread(&StdOldHostMgr::run, this);

    return true;
}

bool StdOldHostMgr::doClose()
{
    myCv_.notify_all();
    myThread_->join();

    return true;
}

void StdOldHostMgr::run()
{
    DLOG(INFO) << "beg";

    while (active()) {
        {
            std::lock_guard<std::mutex> lock(lhm_->hosts_.m_);
            struct timeval now;
            gettimeofday(&now, NULL);

            for(StdHostMap::iterator iter = lhm_->hosts_.begin(); iter != lhm_->hosts_.end(); ++iter)
            {
                if(iter->second.lastAccess_ + scanStartTimeout_ < now.tv_sec)
                {
                    std::lock_guard<std::mutex> inLock(sastm_.m_);
                    StdActiveScanThreadMap::iterator it = sastm_.find(iter->second.mac_);
                    if (it == sastm_.end()) {
                        StdActiveScanThread* sast = new StdActiveScanThread(this, &iter->second);
                        sastm_.insert(std::pair<Mac, StdActiveScanThread*>(iter->second.mac_, sast));
                    }
                }
            }
        }

        std::unique_lock<std::mutex> lock(myMutex_);
        if(myCv_.wait_for(lock,std::chrono::milliseconds(sendSleepTime_)) == std::cv_status::no_timeout) break;
    }

    DLOG(INFO) << "end";
}

void StdOldHostMgr::load(Json::Value &json)
{
    json["checkSleepTime"] >> checkSleepTime_;
    json["scanStartTimeout"] >> scanStartTimeout_;
    json["sendSleepTime"] >> sendSleepTime_;
    json["deleteTimeout"] >> deleteTimeout_;
}

void StdOldHostMgr::save(Json::Value &json)
{
    json["checkSleepTime"] << checkSleepTime_;
    json["scanStartTimeout"] << scanStartTimeout_;
    json["sendSleepTime"] << sendSleepTime_;
    json["deleteTimeout"] << deleteTimeout_;
}
