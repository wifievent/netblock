#ifndef DINFO_H
#define DINFO_H

#include <QString>

class dInfo
{
public:
    dInfo();
    int host_id;
    QString oui;
    QString mac;
    QString last_ip;
    QString name;
    int vectorID;
    bool is_connect = false;
};

#endif // DINFO_H
