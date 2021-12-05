#pragma once

#include "http.h"

class HTTPRequest
{
    Method method_;
    std::string url_;
    Protocol protocol_;
    std::string hostname_;
    std::string useragent_;

    std::vector<std::pair<std::string, std::string>> headers_;
    std::string body_;
    std::string requestpacket_;

    public:
        HTTPRequest();
        ~HTTPRequest();
        void addRequestPacket(char* buf, int& len);
        void addRequestBody(std::string& str);

        int setMethod(Method argmethod);
        Method getMethod();
        int setURL(std::string argurl);
        std::string getURL();
        int setProtocol(Protocol argprotocol);
        Protocol getProtocol();
        int setUserAgent(std::string arguseragent);
        std::string getUserAgent();
        int setHTTPHeader(std::string name, std::string content);
        std::string getHTTPHeader(std::string name);
        int setHTTPHeaderVector(std::vector<std::pair<std::string, std::string>>* headervector);
        std::vector<std::pair<std::string, std::string>>* getHTTPHeaderVector();
        int setRequestBody(std::string argbody);
        std::string getRequestBody();

        int parseRequestPacket();
        int makeRequest();
        size_t getRequestSize();
        std::string* getRequestData();
        std::string updateCursor(size_t& cursorbegin, size_t& cursorend, std::string target, std::string obj, size_t next);
};
