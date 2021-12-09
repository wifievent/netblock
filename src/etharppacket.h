#pragma once

#include "ethhdr.h"
#include "arphdr.h"

#pragma pack(push, 1)
struct EthArpPacket {
    EthHdr ethHdr_;
    ArpHdr arpHdr_;
};
#pragma pack(pop)
