#include "weuiserver.h"

WEUIServer::WEUIServer() {
}

void WEUIServer::handleClnt(TcpClientSocket *clntsock) {

    char buffer[BUFSIZE];
    int len;

    while((len = clntsock->recv(buffer, BUFSIZE)) != -1) {
        if(len == 0) {
            spdlog::info("clntsock is shutdown");
            return;
        }
        uirequest_.addRequestPacket(buffer, len);
        uirequest_.parseRequestPacket();
        spdlog::info("recv data from client\n{}", *(uirequest_.getRequestData()));
        setHttpResponse();
        clntsock->send((char*)uiresponse_.getResponseData()->c_str(), uiresponse_.getResponseSize());
    }
    return;
}

void WEUIServer::setHttpResponse() {
    int size = getWebUIData();

    std::string len = std::to_string(size);

    std::vector<std::pair<std::string, std::string>> headervector;
    headervector.push_back(std::make_pair("Connection", "keep-alive"));
    headervector.push_back(std::make_pair("Content-Length", len.c_str()));
    headervector.push_back(std::make_pair("Content-Type", "text/html"));
    headervector.push_back(std::make_pair("Date", getDateTime()));
    headervector.push_back(std::make_pair("Keep-Alive", "timeout=5, max=100"));
    headervector.push_back(std::make_pair("Server", "WEUIServer"));

    uiresponse_.setProtocol(HTTP1_1);
    uiresponse_.setStatusCode(200);
    uiresponse_.setReasonPhrase();
    uiresponse_.setHTTPHeaderVector(&headervector);
    uiresponse_.setResponseBody(ui_);
    spdlog::info("make Response packet\n{}", *(uiresponse_.getResponseData()));
    uiresponse_.makeResponse();
}

int WEUIServer::getWebUIData() {
    std::ifstream fin("index.html");
    int size = 0;

    if(fin.is_open()){
        fin.seekg(0, std::ios::end);
        size = fin.tellg();
        fin.seekg(0, std::ios::beg);
        fin.read(ui_, size);
        spdlog::info("HTML file contents\n{}", ui_);
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
