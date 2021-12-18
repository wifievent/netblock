#pragma once
#include "tcpserver.h"
#include "httprequest.h"
#include "httpresponse.h"

#include "dinfo.h"
#include "netblock.h"
#include "dbconnect.h"
#include "appjson.h"

#include <fstream>
#include <ctime>
#include <cstdio>

class WEUIServer : public TcpServer
{
    HTTPRequest uirequest_;
    HTTPResponse uiresponse_;
    char ui_[BUFSIZE];
    int statusCode_ = 200;

public:
    StdDInfoList* pDInfoList_;
    NetBlock* pNetblock_;
    DBConnect* nbConnect_;
    std::string rootdir_;

public:
    WEUIServer();
    ~WEUIServer() {}

protected:
    void handleClnt(TcpClientSocket* clntsock) override;
    void setHttpResponse(std::string path);
    int getWebUIData(std::string path);
    std::string getDateTime();
};