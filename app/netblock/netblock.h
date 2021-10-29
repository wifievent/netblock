#include <GApp>
#include <GJson>
#include <GPcapDevice>
#include <QtSql>
#include "livehostmgr.h"

struct NetBlock : GStateObj {
    Q_OBJECT
    Q_PROPERTY(int sendSleepTime MEMBER sendSleepTime_)
    Q_PROPERTY(int nbUpdateTime MEMBER nbUpdateTime_)

    int sendSleepTime_{50}; // 50 msecs
    int nbUpdateTime_{6000}; // 1 minutes

private:
    LiveHostMgr lhm_;
	GPcapDevice device_;
    GIntf* intf_;

    GWaitEvent we_;
    
    GMac gatewayMac_;
    GMac myMac_{GMac::nullMac()};
    GIp myIp_;

    QSqlDatabase ouiDB_;
    QSqlDatabase nbDB_;

    HostMap nbHosts_;
    HostMap nbNewHosts_;

public:
    NetBlock(QObject* parent = nullptr);
    ~NetBlock();

    void dbCheck();

    void updateHosts();

public slots:
    void captured(GPacket* packet);

protected:
	bool doOpen() override;
	bool doClose() override;

    bool sendFindGatewayPacket();
    void run();
    void block();
    void sendInpect(Host host);
    void sendRecover(Host host);
};
