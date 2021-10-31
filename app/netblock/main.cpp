#include "netblock.h"
#include "widget.h"

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

	Widget w;
	w.show();
    a.exec();
    nb.close();
}
