#ifndef WEUDPCLIENT_H
#define WEUDPCLIENT_H

#include <GNetInfo>
#include <GIp>

#include "udpclient.h"

class WEUdpClient : public UdpClient
{
    char sendbuf_[BUFSIZE];
    char recvbuf_[BUFSIZE];

public:
    WEUdpClient() {}
    ~WEUdpClient() {}

protected:
    void setSockAddrBroadcast(int port);

public:
    bool searchProduct(int port, int sec, int millisec, std::string protocol);
};

#endif // WEUDPCLIENT_H
