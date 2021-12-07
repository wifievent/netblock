#include "mainwindow.h"
#include "weudpserver.h"
#include "weudpclient.h"
#include "weuiserver.h"

#include <GApp>

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
    GApp a(argc, argv);
	qDebug() << "NetBlock Started" << version();

    WEUdpClient client;
    if(client.searchProduct(7284, 1, 0, "run already?"))
    {
        qDebug() << "NetBlock stop thanks to using";
        return 0;
    }
    else
    {
        qInfo() << "NetBlock Started" << version();
    }

    WEUdpServer ws;
    ws.start(7284);

    WEUIServer wus;
    wus.rootdir_ = "./test";
    wus.start(80);

    QIcon icon(":/image/logo/logo.ico");
    a.setWindowIcon(icon);

    MainWindow m;
    if (!m.openCheck)
    {
        qDebug() << QString("NB Do not open");
    }
    else
    {
        m.show();
        a.exec();
    }

    ws.stop();
    wus.stop();
}
