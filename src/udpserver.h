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

    bool bind(int port);

protected:
    int setSockOptforReuse();

};

#endif // UDPSERVER_H
