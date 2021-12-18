#include "policyobj.h"

PolicyObj::PolicyObj() {}

PolicyObj::~PolicyObj() {}

void PolicyObj::reset()
{
    policyId = 0;
    startTime = nullptr;
    endTime = nullptr;
    dayOfTheWeek = -1;
    hostId = 0;
};

void PolicyObj::set(std::vector<std::string> row)
{
    policyId = std::stoi(row[0]);
    startTime = row[1];
    endTime = row[2];
    dayOfTheWeek = std::stoi(row[3]);
    hostId = std::stoi(row[4]);
}

int PolicyObj::getPolicyId()
{
    return policyId;
}

std::string PolicyObj::getStartTime()
{
    return startTime;
}

std::string PolicyObj::getEndTime()
{
    return endTime;
}

int PolicyObj::getDayOfTheWeek()
{
    return dayOfTheWeek;
}

int PolicyObj::getHostId()
{
    return hostId;
}
