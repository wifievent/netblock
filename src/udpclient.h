#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include "udpsocket.h"
#include <string>

class UdpClient : public UdpSocket
{
public:
    UdpClient() {}
    ~UdpClient() {}

public:
    bool setSocketBroadcastable();
    void setSockAddr(std::string ip, int port);
    void setRecvTimeout(int sec, int millisec);
};

#endif // UDPCLIENT_H
