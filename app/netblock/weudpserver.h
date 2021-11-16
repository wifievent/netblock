#ifndef WEUDPSERVER_H
#define WEUDPSERVER_Hs

#include "udpserver.h"

class WEUdpServer : public UdpServer
{
public:
    WEUdpServer() {}
    ~WEUdpServer() {}

    bool check = true;

    std::thread* t1;

    void start(int port);
    void stop();

    void handleCnt();
};

#endif // WEUDPSERVER_H
