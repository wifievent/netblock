#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include "udpsocket.h"
#include <QDebug>

class UdpClient : public UdpSocket
{
public:
    UdpClient();
};

#endif // UDPCLIENT_H
