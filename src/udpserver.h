#ifndef UDPSERVER_H
#define UDPSERVER_H

#include "udpsocket.h"
#include <thread>
#include <QDebug>

class UdpServer : public UdpSocket
{
public:
    UdpServer();
    ~UdpServer();

    bool check = true;

    std::thread* t1;

public:
    bool bind(int port);
    void start();
    void stop();

    void handleCnt();

protected:
    int setSockOptforReuse();

};

#endif // UDPSERVER_H
