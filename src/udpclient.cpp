#include "udpclient.h"

bool UdpClient::setSocketBroadcastable()
{
    int optval = 1;
#ifdef Q_OS_WIN
    int res = setsockopt(sock_, SOL_SOCKET, SO_BROADCAST, (char *) &optval, sizeof(optval));
#endif
#ifdef Q_OS_LINUX
    int res = setsockopt(sock_, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
#endif
    if (res == -1)
    {
        perror("setsockopt");
        return false;
    }
    return true;
}

void UdpClient::setSockAddr(std::string ip, int port)
{
    memset(&sockAddr_, 0, sizeof(sockAddr_));
    sockAddr_.sin_family = AF_INET;
    sockAddr_.sin_port = htons(port);
    sockAddr_.sin_addr.s_addr = inet_addr(ip.c_str());
    memset(&sockAddr_.sin_zero, 0, sizeof(sockAddr_.sin_zero));
}

void UdpClient::setRecvTimeout(int sec, int millisec)
{
    struct timeval optVal = {sec, millisec}; //sec, millisec
    int optLen = sizeof(optVal);
#ifdef Q_OS_WIN
    setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, (char *) &optVal, optLen);
#endif
#ifdef Q_OS_LINUX
    setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, &optVal, optLen);
#endif
}
