#include "mainwindow.h"

#include <GApp>

int main(int argc, char *argv[])
{
	GApp a(argc, argv);

    QIcon icon(":/image/logo/logo.ico");

    MainWindow m;
    if(!m.openCheck) {
        qDebug() << QString("NB Do not open");
    }
    else {
        m.show();
        a.exec();
    }
}
