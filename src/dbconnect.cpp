#include "dbconnect.h"

#include <glog/logging.h>

bool DBConnect::open()
{
    rc_ = sqlite3_open(dbName_.data(), &db);
    if(rc_ != SQLITE_OK) {
        DLOG(ERROR) << "Can't open database:" << sqlite3_errmsg(db);
        sqlite3_close(db);
        return false;
    }
    return true;
}

bool DBConnect::close()
{
    sqlite3_close(db);
    return true;
}

//  db_select 함수
std::list<DataList> DBConnect::selectQuery(std::string query)
{
    DLOG(INFO) << "query: " << query;
    /*
    query: SELECT 쿼리문
    return: Select 결과가 담긴 list<Data_List>
    */
    char* err_msg = 0;    //  에러 메시지 저장 변수

        std::list<DataList> dl;    //  select 결과 저장 list

        {
            std::lock_guard<std::mutex> lock(m_);
            //  쿼리 날려서 결과 얻기
            rc_ = sqlite3_exec(db, query.data(), callback, &dl, &err_msg);

            if(rc_ != SQLITE_OK)
            {
                DLOG(ERROR) << "Failed to select data";
                DLOG(ERROR) << "SQL error:" << err_msg;
                sqlite3_free(err_msg);
            }
        }

        return dl;
}

//  select에서 사용하는 callback함수
int DBConnect::callback(void* dl, int argc, char** argv, char** column) {
    //  결과를 저장할 변수
    DataList data;
    data.argc_ = argc;
    for(int i = 0; i < argc; ++i) {
        DLOG(INFO) << "argc: " << argc;
        DLOG(INFO) << "i: " << i;
        data.argv_.push_back(std::string(argv[i]) + "\0");
        data.column_.push_back(std::string(column[i]) + "\0");
    }
    //  결과를 저장
    std::list<DataList>* data_list = (std::list<DataList>*)dl;
    data_list->push_back(data);
    return 0;
}

//  send_query 함수
int DBConnect::sendQuery(std::string query) {
    /*
    query: 쿼리문 dsfsd
    */
    char* err_msg = 0;    //  에러 메시지 저장 변수
    int result = 0;

    {
        std::lock_guard<std::mutex> lock(m_);
        //  쿼리 날리기
        rc_ = sqlite3_exec(db, query.data(), 0, 0, &err_msg);
        if(rc_ != SQLITE_OK) {
            DLOG(ERROR) << "Failed to send query";
            DLOG(ERROR) << "SQL error:" << err_msg;
            sqlite3_free(err_msg);
            result = -1;
        }
    }

    return result;    //  success: 0, fail: -1
}
