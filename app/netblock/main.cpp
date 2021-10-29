#include "netblock.h"
#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

    NetBlock nb;
    nb.open();
    nb.dbCheck();

	Widget w;
	w.show();
	return a.exec();
}
