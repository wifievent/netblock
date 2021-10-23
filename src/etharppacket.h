#pragma once

#include <GEthHdr>
#include <GArpHdr>

#pragma pack(push, 1)
struct EthArpPacket {
	GEthHdr ethHdr_;
	GArpHdr arpHdr_;
};
#pragma pack(pop)
