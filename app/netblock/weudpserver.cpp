#include "weudpserver.h"

void WEUdpServer::start(int port) {

    bind(port);

    t1 = new std::thread(&WEUdpServer::handleCnt, this);
}

void WEUdpServer::stop() {
    check = false;

    t1->join();

    disconnect();
}

void WEUdpServer::handleCnt() {
    char buf[BUFSIZ];
    while(check) {
        int res = recv(buf, sizeof(buf));
        if(res > 0) {

            qDebug() << "res" << res;

            qDebug() << "buf: " << buf;

            if(strcmp(buf, "run already?") == 0) {
                send("run already!", strlen("run already!") + 1);
            }
        }
    }
}
