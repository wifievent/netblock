#ifndef WEUDPSERVER_H
#define WEUDPSERVER_Hs

#include "udpserver.h"

class WEUdpServer : public UdpServer
{
public:
    WEUdpServer() {}
    ~WEUdpServer() {}

    void handleClnt() override;
};

#endif // WEUDPSERVER_H
