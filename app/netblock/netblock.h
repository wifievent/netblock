#include <GApp>
#include <GJson>
#include <GPcapDevice>
#include <QtSql>
#include <QMutexLocker>
#include <QMutex>
#include "livehostmgr.h"

struct LockableSqlDatabase : QSqlDatabase {
    QMutex m_;
    static QSqlDatabase addDatabase(QSqlDriver* driver,
                                    const QString& connectionName = QLatin1String(defaultConnection)) {
        return QSqlDatabase::addDatabase(driver, connectionName);
    }
};

struct NetBlock : GStateObj {
    Q_OBJECT
    Q_PROPERTY(int sendSleepTime MEMBER sendSleepTime_)
    Q_PROPERTY(int nbUpdateTime MEMBER nbUpdateTime_)

    int sendSleepTime_{50}; // 50 msecs
    int nbUpdateTime_{60000}; // 1 minutes

private:
	GPcapDevice device_;
    GIntf* intf_;

    GWaitEvent we_;
    
    GMac gatewayMac_{GMac::nullMac()};
    GMac myMac_{GMac::nullMac()};
    GIp myIp_;

    HostMap nbHosts_;
    HostMap nbNewHosts_;

    bool dbCheck();

public:
    NetBlock(QObject* parent = nullptr);
    ~NetBlock();

    void updateHosts();

    LiveHostMgr lhm_;

    QMutex nbDBLock_;
    QMutex ouiDBLock_;

    QSqlDatabase nbDB_;
    QSqlDatabase ouiDB_;

public slots:
    void captured(GPacket* packet);
    void block();

protected:
	bool doOpen() override;
	bool doClose() override;

    void sendInfect(Host host);
    void sendRecover(Host host);

    void run();
    struct MyThread: GThread {
        MyThread(QObject *parent) : GThread(parent) {}
        ~MyThread() {}
        void run() override {
            NetBlock* netBlock = dynamic_cast<NetBlock*>(parent());
            netBlock->run();
        }
    } thread_{this};

public:
    void propLoad(QJsonObject jo) override;
    void propSave(QJsonObject& jo) override;
};
