#include "weudpclient.h"

bool WEUdpClient::searchProduct(int port, int sec, int millisec, std::string protocol)
{
    strncpy(sendbuf_, protocol.c_str(), protocol.length());
    setSocketBroadcastable();
    setRecvTimeout(sec, millisec);
    setSockAddrBroadcast(port);
    send(sendbuf_, strlen(sendbuf_) + 1);
    if(recv(recvbuf_, BUFSIZE) != -1)
    {
        if(strcmp(recvbuf_, "run already!") == 0)
        {
            DLOG(INFO) << "There is already wifievent product running...";
            return true;
        }
    }
    DLOG(INFO) << "There is no wifevent product...";
    return false;
}

void WEUdpClient::setSockAddrBroadcast(int port)
{
    //myIp or !netmask
    RtmEntry* entry = NetInfo::instance().rtm().getBestEntry(Ip("8.8.8.8"));
    Ip broadcastIp = entry->intf()->gateway() | ~(entry->intf()->mask());
    DLOG(INFO) << "gateway ip:" << std::string(entry->intf()->gateway()).data();
    DLOG(INFO) << "mask:" << std::string(entry->intf()->mask()).data();
    DLOG(INFO) << "broadcast ip:"<<std::string(broadcastIp).data();
    setSockAddr(std::string(broadcastIp), port);
    return;
}
