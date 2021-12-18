#pragma once

#include "netinfo.h"
#include "ip.h"

#include "udpclient.h"
#include <QDebug>

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

