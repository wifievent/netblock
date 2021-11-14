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

void UdpServer::start()
{
    bind(7284);

    t1 = new std::thread(&UdpServer::handleCnt, this);
}

void UdpServer::stop()
{
    check = false;

    t1->join();

    disconnect();
}

void UdpServer::handleCnt() {
    char buf[BUFSIZ];
    while(check) {
        int res = recv(buf, sizeof(buf));
        if(res == -1) {
            continue;
        }

        qDebug() << "buf: " << buf;

        if(strcmp(buf, "run already?") == 0) {
            send("run already!", strlen("run already!") + 1);
        }
    }
}

int UdpServer::setSockOptforReuse()
{
    int optval = 1;
    int result = 1;
    result = setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    return result; //success 0, fail -1
}
