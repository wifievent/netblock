#ifndef WEUDPSERVER_H
#define WEUDPSERVER_Hs

#include "udpserver.h"
#include <QtDebug>

class WEUdpServer : public UdpServer
{
public:
    WEUdpServer() {}
    ~WEUdpServer() {}

    void handleClnt() override;
};

#endif // WEUDPSERVER_H
