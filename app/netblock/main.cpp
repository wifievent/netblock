#include "weudpserver.h"
#include "weudpclient.h"


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

    DLOG(INFO) << "netblock";
    NetBlock netblock;
//    Json::Value jv;
//    if(AppJson::loadFromFile("netblock.json", jv))
//    {
//        jv["NetBlock"] >> netblock;
//        jv["NetBlock"] << netblock;
//    }

    if(!netblock.open())
    {
        DLOG(ERROR) << "NB Do not open";
        return 0;
    }

//    MainWindow m;
//    if (!m.openCheck)
//    {
//        qDebug() << QString("NB Do not open");
//    }
//    else
//    {
//        m.show();
//        a.exec();
//    }

    while(true) {

    }

    netblock.close();

    ws.stop();

    return 0;
}
