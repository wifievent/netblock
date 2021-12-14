#include "weudpserver.h"
#include "weudpclient.h"

#include <QApplication>

#include <iostream>

#include "netblock.h"

#include <glog/logging.h>

const char *version()
{
    return
#ifdef _DEBUG
#include "../../version.txt"
        " Debug Build(" __DATE__ " " __TIME__ ")";
#else // RELEASE
#include "../../version.txt"
        " Release Build(" __DATE__ " " __TIME__ ")";
#endif // _DEBUG
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DLOG(INFO) << "NetBlock Started" << version();

    WEUdpClient client;
    if(client.searchProduct(7284, 1, 0, "run already?"))
    {
        DLOG(ERROR) << "NetBlock stop tanks to using";
        qDebug() << "NetBlock stop thanks to using";
        return 0;
    }

    WEUdpServer ws;
    ws.start(7284);

    NetBlock nb;
    if(nb.open())
    {

    }
    else
    {
        DLOG(ERROR) << "NB don't open";
    }

    ws.stop();

    return 0;
}
