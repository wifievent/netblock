#ifndef WEUDPSERVER_H
#define WEUDPSERVER_Hs

#include "udpserver.h"
#include <QtDebug>

class WEUdpServer : public UdpServer
{
public:
    WEUdpServer() {}
    ~WEUdpServer() {}

    bool check = true;

    std::thread* t1;

    void start(int port);
    void stop();

    void handleClnt() override;
};

#endif // WEUDPSERVER_H
