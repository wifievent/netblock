#include "mainwindow.h"

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
}
