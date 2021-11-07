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

void PolicyObj::set(QVector<QString> row)
{
    policyId = row[0].toInt();
    startTime = row[1];
    endTime = row[2];
    dayOfTheWeek = row[3].toInt();
    hostId = row[4].toInt();
}

int PolicyObj::getPolicyId()
{
    return policyId;
}

QString PolicyObj::getStartTime()
{
    return startTime;
}

QString PolicyObj::getEndTime()
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
