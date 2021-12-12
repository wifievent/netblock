#include "weudpserver.h"

void WEUdpServer::handleClnt() {
    DLOG(INFO) << "start handleClnt";
    char buf[BUFSIZ];
    while(true) {
        int res = recv(buf, sizeof(buf));
        if(res > 0) {
            DLOG(INFO) << "buf: %s" << buf;

            if(strcmp(buf, "run already?") == 0) {
                send("run already!", strlen("run already!") + 1);
            }
        } else {
            break;
        }
    }
}
