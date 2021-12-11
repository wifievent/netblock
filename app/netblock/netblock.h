#include "livehostmgr.h"

#include <thread>
#include <mutex>

#include "appjson.h"
#include "stateobj.h"
#include "pcapdevice.h"

#include "dbconnect.h"

struct NetBlock : StateObj {
    int sendSleepTime_{50}; // 50 msecs
    int nbUpdateTime_{60000}; // 1 minutes
    int infectSleepTime_{10000}; //  10 sec

private:
    PcapDevice device_;
    Intf* intf_;
    
    Mac gatewayMac_{Mac::nullMac()};
    Mac myMac_{Mac::nullMac()};
    Ip myIp_;

    StdHostMap nbHosts_;
    StdHostMap nbNewHosts_;

    bool dbCheck();

public:
    NetBlock() {};
    ~NetBlock() { close(); };

    void updateHosts();

    StdLiveHostMgr lhm_{&device_};

    DBConnect* nbConnect_;
    DBConnect* ouiConnect_;

public slots:
    void captured(Packet* packet);
    void block();

protected:
	bool doOpen() override;
	bool doClose() override;

    void findGatewayMac();

    void sendInfect(StdHost host);
    void sendRecover(StdHost host);

    void run();

    std::mutex blockMutex_;
    std::condition_variable blockCv_;

    std::thread* captureThread_;
    std::mutex captureMutex_;
    std::condition_variable captureCv_;
    void capture();

    std::thread* dbUpdateThread_;
    std::mutex dbMutex_;
    std::condition_variable dbCv_;
    void dbUpdateRun();

    std::thread* infectThread_;
    std::mutex infectMutex_;
    std::condition_variable infectCv_;
    void infectRun();

public:
    void load(Json::Value& json) override;
    void save(Json::Value& json) override;
};
