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
            qDebug() << "There is already wifievent product running...";
            return true;
        }
    }
    qDebug() << "There is no wifievent product...";
    return false;
}

void WEUdpClient::setSockAddrBroadcast(int port)
{
    //myIp or !netmask
    GRtmEntry* entry = GNetInfo::instance().rtm().getBestEntry(QString("8.8.8.8"));
    GIp broadcastip = entry->intf()->gateway() | ~(entry->intf()->mask());
    qInfo() << "gateway ip:" << QString(entry->intf()->gateway());
    qInfo() << "mask:" << QString(entry->intf()->mask());
    qInfo() << "broadcast ip:" << QString(broadcastip);
    setSockAddr(QString(broadcastip).toStdString(), port);
    return;
}
