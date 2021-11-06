#ifndef POLICYOBJ_H
#define POLICYOBJ_H

#include <QtCore>

class PolicyObj
{
protected:
    int policyId;
    QString startTime;
    QString endTime;
    int dayOfTheWeek;
    int hostId;

public:
    PolicyObj();
    ~PolicyObj();

    void reset();
    void set(QVector<QString> row);
    int getPolicyId();
    QString getStartTime();
    QString getEndTime();
    int getDayOfTheWeek();
    int getHostId();
};

#endif // POLICYOBJ_H
