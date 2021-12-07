#include <GApp>
#include <GJson>
#include <GPcapDevice>
#include <QtSql>
#include <QMutexLocker>
#include <QMutex>
#include "livehostmgr.h"


#include <thread>
#include <mutex>

#include "stateobj.h"
#include "pcapdevice.h"

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
    Intf* intf_;

    GWaitEvent we_;
    
    Mac gatewayMac_{Mac::nullMac()};
    Mac myMac_{Mac::nullMac()};
    Ip myIp_;

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
    void captured(Packet* packet);
    void block();

protected:
	bool doOpen() override;
	bool doClose() override;

    void sendInfect(Host host);
    void sendRecover(Host host);

    void run();

    std::thread* dbUpdateThread_;
    std::mutex dbMutex_;
    std::condition_variable dbCv_;
    void dbUpdateRun();

    std::thread* infectThread_;
    std::mutex infectMutex_;
    std::condition_variable infectCv_;
    void infectRun();

public:
    void propLoad(QJsonObject jo) override;
    void propSave(QJsonObject& jo) override;
};
