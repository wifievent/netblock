#include "weudpserver.h"
#include "weudpclient.h"
#include "weuiserver.h"

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

int main()
{
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


    WEUIServer wus;
    wus.rootdir_ = "./nbui";
    wus.start(80);

    NetBlock netblock;
    if(netblock.open())
    {
        DLOG(INFO) << "NetBlock open";

        wus.pDInfoList_ = &netblock.lhm_.dInfoList_;
        wus.pNetblock_ = &netblock;
        wus.nbConnect_ = netblock.nbConnect_;

        while(true)
        {
            sleep(100000);
        }
    }
    else
    {
        DLOG(ERROR) << "NetBlock don't open";
    }

    wus.stop();
    ws.stop();

    return 0;
}
