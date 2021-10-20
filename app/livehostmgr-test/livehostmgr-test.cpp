#include <GApp>
#include <GJson>
#include "livehostmgr.h"
#include "livehostmgr-test.h"

int main(int argc, char *argv[]) {
	GApp a(argc, argv);

	LiveHostMgr lhm;
	QJsonObject jo = GJson::loadFromFile();
	jo["lhm"] >> lhm;
	jo["lhm"] << lhm;
	GJson::saveToFile(jo);

	Obj obj;
	QObject::connect(&lhm, &LiveHostMgr::hostAdded, &obj, &Obj::processhostAdded, Qt::BlockingQueuedConnection);
	QObject::connect(&lhm, &LiveHostMgr::hostDeleted, &obj, &Obj::processhostDeleted, Qt::BlockingQueuedConnection);

	if (!lhm.open()) {
		qWarning() << QString("lhm.open() return false %1").arg(lhm.err->msg());
		return -1;
	}

	int res = a.exec();
	lhm.close();
	return res;
}
