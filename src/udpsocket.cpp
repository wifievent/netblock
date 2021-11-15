#include "udpsocket.h"

UdpSocket::UdpSocket()
{
    if((sock_=socket(PF_INET, SOCK_DGRAM, 0)) < 0)
       {
           qDebug() << "socket create failed";
       }
}

UdpSocket::~UdpSocket()
{
    disconnect();
}

int UdpSocket::send(char *buf, size_t len)
{
    return ::sendto(sock_, buf, len, 0, (struct sockaddr*)&sockAddr_, sizeof(sockAddr_));
}

int UdpSocket::recv(char *buf, size_t len)
{
    socklen_t sockLen = sizeof(sockAddr_);
//    memset(&sockAddr_, 0, sizeof(sockAddr_));
    memset(buf, 0, len);
    ssize_t res = ::recvfrom(sock_, buf, len, MSG_DONTWAIT, (struct sockaddr*)&sockAddr_, &sockLen);
    return res;
}

int UdpSocket::disconnect()
{
    int result = 0;
    if(sock_ != 0)
    {
        ::shutdown(sock_, SHUT_RDWR);
        result = ::close(sock_);
        sock_ = 0;
    }
    return result;
    //success 0, fail -1
}
