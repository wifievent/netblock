#pragma once

#include <GEthHdr>
#include <GArpHdr>

#include "ethhdr.h"
#include "arphdr.h"

#pragma pack(push, 1)
struct EthArpPacket {
    EthHdr ethHdr_;
    ArpHdr arpHdr_;
};
#pragma pack(pop)
