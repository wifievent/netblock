#include <GApp>
#include <GJson>
#include <GPcapDevice>
#include <QtSql>
#include <QMutexLocker>
#include <QMutex>
#include "livehostmgr.h"


#include <pcapdevice.h>

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
    Q_PROPERTY(int infectSleepTime MEMBER infectSleepTime_)

    int sendSleepTime_{50}; // 50 msecs
    int nbUpdateTime_{60000}; // 1 minutes
    int infectSleepTime_{10000}; //  10 sec

private:
    PcapDevice device_;
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

	LiveHostMgr lhm_{this, &device_};

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

    struct DBUpdateThread: GThread {
        DBUpdateThread(QObject *parent) : GThread(parent) {}
        ~DBUpdateThread() {}
        GWaitEvent we_;
        void run() override;
    } dbUpdateThread_{this};

    struct InfectThread: GThread {
        InfectThread(QObject *parent) : GThread(parent) {}
        ~InfectThread() {}
        GWaitEvent we_;
        void run() override;
    } infectThread_{this};

public:
    void propLoad(QJsonObject jo) override;
    void propSave(QJsonObject& jo) override;
};
