#ifndef POLICYOBJ_H
#define POLICYOBJ_H

#include <QtCore>

#include <string>
#include <vector>

class PolicyObj
{
protected:
    int policyId;
    std::string startTime;
    std::string endTime;
    int dayOfTheWeek;
    int hostId;

public:
    PolicyObj();
    ~PolicyObj();

    void reset();
    void set(std::vector<std::string> row);
    int getPolicyId();
    std::string getStartTime();
    std::string getEndTime();
    int getDayOfTheWeek();
    int getHostId();
};

#endif // POLICYOBJ_H
