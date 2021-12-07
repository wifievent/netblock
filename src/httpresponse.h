#pragma once

#include "http.h"

class HTTPResponse
{
    Protocol protocol_;
    size_t statuscode_;
    std::string reasonphrase_;
    std::vector<std::pair<std::string, std::string>> headers_;
    std::string body_;
    std::string responsepacket_;

    public:
        HTTPResponse();
        ~HTTPResponse();
        void addResponsePacket(char* buf, int& len);

        int setProtocol(Protocol argprotocol);
        Protocol getProtocol();
        int setStatusCode(size_t argstatuscode);
        size_t getStatusCode();
        int setReasonPhrase();
        std::string getReasonPhrase();
        int setHTTPHeader(std::string name, std::string content);
        std::string getHTTPHeader(std::string name);
        int setHTTPHeaderVector(std::vector<std::pair<std::string, std::string>>* headervector);
        std::vector<std::pair<std::string, std::string>>* getHTTPHeaderVector();
        int setResponseBody(std::string argbody);
        std::string getResponseBody();

        int makeResponse();
        int parseResponsePacket();
        size_t getResponseSize();
        std::string* getResponseData();
        std::string updateCursor(size_t& cursorbegin, size_t& cursorend, std::string target, std::string obj, size_t next);
        void resetData();
};
