#include "weuiserver.h"

void operator << (Json::Value& json, bool& v)
{
    json = v;
}

WEUIServer::WEUIServer() {
}

void WEUIServer::handleClnt(TcpClientSocket *clntsock) {

    char buffer[BUFSIZE];
    int len;
    while((len = clntsock->recv(buffer, BUFSIZE)) != -1) {
        if(len == 0) {
            DLOG(ERROR) << "Clntsock is shutdown";
            break;
        }
        uirequest_.resetData();
        uirequest_.addRequestPacket(buffer, len);
        uirequest_.parseRequestPacket();
        setHttpResponse(uirequest_.getURL());
        clntsock->send((char*)uiresponse_.getResponseData()->c_str(), uiresponse_.getResponseSize());
    }
}

void WEUIServer::setHttpResponse(std::string path) {
    uiresponse_.resetData();
    int size = 0;
    DLOG(INFO) << "request path: " << path;

    if(path == "/") {
        size = getWebUIData("/index.html");
    }
    else {
        size = getWebUIData(path);
    }

    std::string len = std::to_string(size);

    std::vector<std::pair<std::string, std::string>> headervector;
    headervector.push_back(std::make_pair("Connection", "keep-alive"));
    headervector.push_back(std::make_pair("Content-Length", len.c_str()));
    if(path.ends_with(".css"))
    {
        headervector.push_back(std::make_pair("Content-Type", "text/css;charset=UTF-8"));
    }
    else if(path.ends_with(".js"))
    {
        headervector.push_back(std::make_pair("Content-Type", "text/javascript;charset=UTF-8"));
    }
    else
    {
        headervector.push_back(std::make_pair("Content-Type", "text/html"));
    }
    headervector.push_back(std::make_pair("Date", getDateTime()));
    headervector.push_back(std::make_pair("Keep-Alive", "timeout=5, max=100"));
    headervector.push_back(std::make_pair("Server", "WEUIServer"));

    uiresponse_.setProtocol(HTTP1_1);
    uiresponse_.setStatusCode(200);
    uiresponse_.setReasonPhrase();
    uiresponse_.setHTTPHeaderVector(&headervector);
    uiresponse_.setResponseBody(ui_);
    uiresponse_.makeResponse();
    DLOG(INFO) << ui_;
}

int WEUIServer::getWebUIData(std::string path) {
    DLOG(INFO) << "Get local data from: " << rootdir_ + path;

    int size = 0;

    if(path == "/device")
    {
        if(uirequest_.getMethod() == GET)
        {
            Json::Value jv;
            if(pDInfoList_->size() > 0)
            {
                for(StdDInfo dInfo : *pDInfoList_)
                {
                    Json::Value subJv;
                    subJv["host_id"] << dInfo.hostId_;
                    std::string mac = std::string(dInfo.mac_);
                    std::string ip = std::string(dInfo.ip_);
                    subJv["mac"] << mac;
                    subJv["ip"] << ip;
                    subJv["host_name"] << dInfo.hostName_;
                    subJv["nick_name"] << dInfo.nickName_;
                    subJv["oui"] << dInfo.oui_;
                    subJv["isConnect"] << dInfo.isConnect_;
                    jv.append(subJv);
                }
            }
            std::string data = jv.toStyledString();
            strncpy(ui_, data.data(), data.length());
            size = data.length();
        }
        else if(uirequest_.getMethod() == PATCH)
        {
            std::string body = uirequest_.getRequestBody();
            Json::Reader read;
            Json::Value jv;
            read.parse(body, jv);
            std::string hostId, nickName;
            jv["host_id"] >> hostId;
            jv["nick_name"] >> nickName;

            std::string query("UPDATE host SET nick_name=':nick_name' WHERE host_id=:host_id");
            query.replace(query.find(":nick_name"), std::string(":nick_name").length(), nickName);
            query.replace(query.find(":host_id"), std::string(":host_id").length(), hostId);
            int result = nbConnect_->sendQuery(query);
            std::string data = result == 0 ? "success" : "fail";
            strncpy(ui_, data.data(), data.length());
            size = data.length();
        }
        else if(uirequest_.getMethod() == DELETE)
        {
            std::string body = uirequest_.getRequestBody();
            Json::Reader read;
            Json::Value jv;
            read.parse(body, jv);
            std::string hostId;
            jv["host_id"] >> hostId;

            std::string query("DELETE FROM host WHERE host_id=:host_id");
            query.replace(query.find(":host_id"), std::string(":host_id").length(), hostId);
            int result = nbConnect_->sendQuery(query);
            std::string data = result == 0 ? "success" : "fail";
            strncpy(ui_, data.data(), data.length());
            size = data.length();
        }

    } else if(path == "/policy"){
        if(uirequest_.getMethod() == GET)
        {
            std::string body = uirequest_.getRequestBody();
            Json::Reader read;
            Json::Value jv;
            read.parse(body, jv);
            std::string hostId;
            jv["host_id"] >> hostId;

            std::string query("SELECT policy_id, start_time, end_time, day_of_the_week, host_id FROM policy WHERE host_id = :host_id ORDER BY day_of_the_week ASC");
            query.replace(query.find(":host_id"), std::string(":host_id").length(), hostId);
            std::list<DataList> dl = nbConnect_->selectQuery(query);

            if(dl.size() > 0)
            {
                Json::Value policyJv;
                for(DataList data : dl)
                {
                    Json::Value subJv;
                    int policy_id = std::stoi(data.argv_[0]);
                    subJv["policy_id"] << policy_id;
                    subJv["start_time"] << data.argv_[1];
                    subJv["end_time"] << data.argv_[2];
                    int dayOfWeek = std::stoi(data.argv_[3]);
                    subJv["day_of_the_week"] << dayOfWeek;
                    policyJv.append(subJv);
                }
                jv.append(policyJv);
            }

            std::string data = jv.toStyledString();
            strncpy(ui_, data.data(), data.length());
            size = data.length();
        }
        else if(uirequest_.getMethod() == POST)
        {
            int result = 0;
            std::string body = uirequest_.getRequestBody();
            Json::Reader read;
            Json::Value jv;
            read.parse(body, jv);
            std::string hostId;
            jv["host_id"] >> hostId;

            for(Json::Value subJv : jv["policy"])
            {
                std::string sTime;
                subJv["start_time"] >> sTime;
                std::string eTime;
                subJv["end_time"] >> eTime;
                std::string dayOfWeek;
                subJv["day_of_the_week"] >> dayOfWeek;

                std::string query("INSERT INTO policy(host_id, start_time, end_time, day_of_the_week) VALUES(:host_id, ':start_time', ':end_time', :day_of_the_week)");
                query.replace(query.find(":host_id"), std::string(":host_id").length(), hostId);
                query.replace(query.find(":start_time"), std::string(":start_time").length(), sTime);
                query.replace(query.find(":end_time"), std::string(":end_time").length(), eTime);
                query.replace(query.find(":day_of_the_week"), std::string(":day_of_the_week").length(), dayOfWeek);
                result = nbConnect_->sendQuery(query);
                if(result != 0)
                {
                    break;
                }
            }

            std::string data = result == 0 ? "success" : "fail";
            strncpy(ui_, data.data(), data.length());
        }
        else if(uirequest_.getMethod() == PATCH)
        {
            std::string body = uirequest_.getRequestBody();
            Json::Reader read;
            Json::Value jv;
            read.parse(body, jv);
            std::string policyId;
            std::string sTime;
            std::string eTime;
            std::string dayOfWeek;
            jv["policy_id"] >> policyId;
            jv["start_time"] >> sTime;
            jv["end_time"] >> eTime;
            jv["day_of_the_week"] >> dayOfWeek;

            std::string query("UPDATE policy SET start_time=':start_time', end_time=':end_time', day_of_the_week=:day_of_the_week WHERE policy_id=:policy_id");
            query.replace(query.find(":start_time"), std::string(":start_time").length(), sTime);
            query.replace(query.find(":end_time"), std::string(":end_time").length(), eTime);
            query.replace(query.find(":day_of_the_week"), std::string(":day_of_the_week").length(), dayOfWeek);
            query.replace(query.find(":policy_id"), std::string(":policy_id").length(), policyId);
            int result = nbConnect_->sendQuery(query);
            std::string data = result == 0 ? "success" : "fail";
            strncpy(ui_, data.data(), data.length());
        }
        else if(uirequest_.getMethod() == DELETE)
        {
            std::string body = uirequest_.getRequestBody();
            Json::Reader read;
            Json::Value jv;
            read.parse(body, jv);
            std::string policyId;
            jv["policy_id"] >> policyId;

            std::string query("DELETE FROM policy WHERE policy_id = :policy_id");
            query.replace(query.find(":policy_id"), std::string(":policy_id").length(), policyId);
            int result = nbConnect_->sendQuery(query);
            std::string data = result == 0 ? "success" : "fail";
            strncpy(ui_, data.data(), data.length());
        }
    } else if(path=="/page")
    {
        std::string test_body("{\"host_id\": 1, \"policy\":[{\"start_time\": \"1300\", \"end_time\": \"1800\", \"day_of_the_week\": 1}, {\"start_time\": \"1300\", \"end_time\": \"1800\", \"day_of_the_week\": 2}]}");
        Json::Reader read;
        Json::Value jv;
        read.parse(test_body, jv);
        std::string hostId;
        jv["host_id"] >> hostId;

        std::string data = "";

        for(Json::Value subJv : jv["policy"])
        {
            for(auto it = subJv.begin(); it != subJv.end(); ++it)
            {
                DLOG(INFO) << it->toStyledString();
                data += it->toStyledString();
            }
        }

        strncpy(ui_, data.data(), data.length());
        size = data.length();
    } else{
        std::ifstream fin(rootdir_+path);

        if(fin.is_open()){
            fin.seekg(0, std::ios::end);
            size = fin.tellg();
            fin.seekg(0, std::ios::beg);
            fin.read(ui_, size);
        }
    }

    return size;
}

std::string WEUIServer::getDateTime() {
    char date[30] = {'\0'};

    time_t now = time(0);
    tm *gmtm = gmtime(&now);
    char* dt = asctime(gmtm);

    char mon[4];
    char dayofweek[4];
    int year, day, hour, min, sec;

    sscanf(dt, "%s %s %d %d:%d:%d %d\n", dayofweek, mon, &day, &hour, &min, &sec, &year);
    sprintf(date, "%s, %02d %s %d %02d:%02d:%02d GMT", dayofweek, day, mon, year, hour, min, sec);
    std::string dateheader = date;
    return dateheader;
}
