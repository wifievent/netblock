#pragma once

#include <GPcapDevice>
#include <GWaitEvent>

#include <thread>
#include <mutex>
#include "pcapdevice.h"

struct G_EXPORT FullScan : GStateObj {
	Q_OBJECT
	Q_PROPERTY(int sendSleepTime MEMBER sendSleepTime_)
	Q_PROPERTY(int rescanSleepTime MEMBER rescanSleepTime_)

	int sendSleepTime_{50}; // 50 msecs
    int rescanSleepTime_{600000}; // 10 minutes

public:
	FullScan(QObject* parent = nullptr);
	~FullScan() override;

    PcapDevice* device_{nullptr}; // reference
	GWaitEvent we_;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	void run();

    std::thread* myThread_;
    std::mutex myMutex_;
    std::condition_variable myCv_;


	struct MyThread: GThread {
		MyThread(QObject *parent) : GThread(parent) {}
		~MyThread() {}
		void run() override {
			FullScan* fullScan = dynamic_cast<FullScan*>(parent());
			fullScan->run();
		}
	} thread_{this};

};
