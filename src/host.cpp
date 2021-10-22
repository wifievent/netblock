#include "host.h"

Host::Host() {
}

Host::Host(GMac mac, GIp ip) {
	mac_ = mac;
	ip_ = ip;
	name_ = QString(ip);
	lastAccess_ = timer().elapsed();
}

Host::Host(GMac mac, GIp ip, QString name) {
	mac_ = mac;
	ip_ = ip;
	name_ = name;
	lastAccess_ = timer().elapsed();
}
