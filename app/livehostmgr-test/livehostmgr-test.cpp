#include <GApp>
#include <GJson>
#include "livehostmgr.h"
#include "livehostmgr-test.h"

int main(int argc, char *argv[]) {
	GApp a(argc, argv);

	LiveHostMgr lhm;
	QJsonObject jo = GJson::loadFromFile();
	jo["LiveHostMgr"] >> lhm;
	jo["LiveHostMgr"] << lhm;
	GJson::saveToFile(jo);

	Obj obj;
	QObject::connect(&lhm, &LiveHostMgr::hostDetected, &obj, &Obj::processHostDetected, Qt::BlockingQueuedConnection);
	QObject::connect(&lhm, &LiveHostMgr::hostDeleted, &obj, &Obj::processHostDeleted, Qt::BlockingQueuedConnection);

	if (!lhm.open()) {
		qWarning() << QString("lhm.open() return false %1").arg(lhm.err->msg());
		return -1;
	}

	int res = a.exec();
	lhm.close();
	return res;
}
