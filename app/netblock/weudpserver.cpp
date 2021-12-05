#include "weudpserver.h"

void WEUdpServer::handleClnt() {
    char buf[BUFSIZ];
    while(true) {
        int res = recv(buf, sizeof(buf));
        if(res >= 0) {
            spdlog::info("buf: {}", buf);

            if(strcmp(buf, "run already?") == 0) {
                send("run already!", strlen("run already!") + 1);
            }
        }
        else {
            break;
        }
    }
}
