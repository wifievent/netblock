#include "weudpserver.h"

void WEUdpServer::handleClnt() {
    char buf[BUFSIZ];
    while(true) {
        int res = recv(buf, sizeof(buf));
        if(res >= 0) {
            DLOG(INFO) << "buf: " << buf;

            if(strcmp(buf, "run already?") == 0) {
                send("run already!", strlen("run already!") + 1);
            }
        }
        else {
            break;
        }
    }
}
