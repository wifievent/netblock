#include "netblock.h"
#include "widget.h"
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);


    NetBlock nb;

    QJsonObject jo = GJson::loadFromFile();
    jo["NetBlock"] >> nb;
    jo["NetBlock"] << nb;
    GJson::saveToFile(jo);
    nb.open();


    QIcon icon(":/image/logo/logo.ico");

    MainWindow m;
    m.show();
    a.exec();
    nb.close();
}
