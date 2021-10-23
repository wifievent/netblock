#pragma once

#include <GPcapDevice>
#include <GWaitEvent>

struct G_EXPORT FullScan : GStateObj {
	Q_OBJECT
	Q_PROPERTY(int sendSleepTime MEMBER sendSleepTime_)
	Q_PROPERTY(int rescanSleepTime MEMBER rescanSleepTime_)

	int sendSleepTime_{50}; // 50 msec
	int rescanSleepTime_{60000}; // 10 minutes

public:
	FullScan(QObject* parent = nullptr);
	~FullScan() override;

	GPcapDevice* device_{nullptr}; // reference
	GWaitEvent we_;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	void run();

	struct MyThread: QThread {
		MyThread(QObject *parent) : QThread(parent) {}
		~MyThread() {}
		void run() override {
			FullScan* fullScan = dynamic_cast<FullScan*>(parent());
			fullScan->run();
		}
	} thread_{this};
};
