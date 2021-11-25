#include "weudpserver.h"

void WEUdpServer::start(int port) {

    bind(port);

    t1 = new std::thread(&WEUdpServer::handleClnt, this);
}

void WEUdpServer::stop() {
    check = false;

    disconnect();

    t1->join();
}

void WEUdpServer::handleClnt() {
    char buf[BUFSIZ];
    while(check) {
        int res = recv(buf, sizeof(buf));
        if(res > 0) {
            qDebug() << "buf: " << buf;

            if(strcmp(buf, "run already?") == 0) {
                send("run already!", strlen("run already!") + 1);
            }
        }
    }
}
