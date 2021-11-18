#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include <QDebug>

#include "socket.h"

class UdpSocket : public Socket
{
public:
    int sock_{0};
    struct sockaddr_in sockAddr_;

public:
    UdpSocket();
    ~UdpSocket() override;

    int send(char *buf, size_t len) override;
    int recv(char* buf, size_t len) override;
    int disconnect() override;

};

#endif // UDPSOCKET_H
