#include "host.h"

Host::Host() {
}

Host::Host(GMac mac, GIp ip) {
	mac_ = mac;
	ip_ = ip;
	lastAccess_ = timer().elapsed();
}

Host::Host(GMac mac, GIp ip, QString dhcpName) {
	mac_ = mac;
	ip_ = ip;
	dhcpName_ = dhcpName;
	lastAccess_ = timer().elapsed();
}
