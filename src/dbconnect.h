#pragma once

#include <string>
#include <mutex>
#include <list>
#include <vector>

#include "sqlite/sqlite3.h"

struct DataList
{
    int argc_;
    std::vector<std::string> argv_;
    std::vector<std::string> column_;
};

class DBConnect
{
private:
    sqlite3* db;
    std::string dbName_;
public:
    DBConnect(std::string dbName): dbName_(dbName) {};
    ~DBConnect() {};

    std::mutex m_;

    int rc_;

    std::list<DataList> selectQuery(std::string query);
    int sendQuery(std::string query);

    bool open();
    bool close();
protected:
    static int callback(void* dl, int ac, char** av, char** c);
};
