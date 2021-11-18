#include "udpserver.h"

UdpServer::UdpServer()
{
    setSockOptforReuse();
}

UdpServer::~UdpServer()
{

}

bool UdpServer::bind(int port)
{
    memset(&sockAddr_, 0, sizeof(sockAddr_));
    sockAddr_.sin_family=AF_INET;
    sockAddr_.sin_addr.s_addr=INADDR_ANY;
    sockAddr_.sin_port=htons(port);
    if(::bind(sock_, (struct sockaddr*)&sockAddr_, sizeof(sockAddr_))==-1)
    {
        qDebug() << "bind() error";
        return false;
    }
    return true;
}

int UdpServer::setSockOptforReuse()
{
    int optval = 1;
    int result = 1;
#ifdef Q_OS_WIN
    result = setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));
#endif
#ifdef Q_OS_LINUX
    result = setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
#endif
    return result; //success 0, fail -1
}
