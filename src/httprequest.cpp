#include "httprequest.h"

using namespace std;

HTTPRequest::HTTPRequest()
{
    body_ = "";
    requestpacket_ = "";
}

HTTPRequest::~HTTPRequest()
{
}

void HTTPRequest::addRequestPacket(char* buf, int& len)
{
    requestpacket_.append(buf, len);
}

void HTTPRequest::addRequestBody(string& str)
{
    body_ += str;
}

//set, get func begin

int HTTPRequest::setMethod(Method argmethod)
{
    method_ = argmethod;
    return 0;
}

Method HTTPRequest::getMethod()
{
    return method_;
}

int HTTPRequest::setURL(string argurl)
{
    url_ = argurl;
    return 0;
}

string HTTPRequest::getURL()
{
    return url_;
}

int HTTPRequest::setProtocol(Protocol argprotocol)
{
    protocol_ = argprotocol;
    return 0;
}

Protocol HTTPRequest::getProtocol()
{
    return protocol_;
}

int HTTPRequest::setUserAgent(string arguseragent)
{
    useragent_ = arguseragent;
    return 0;
}

string HTTPRequest::getUserAgent()
{
    return useragent_;
}

int HTTPRequest::setHTTPHeader(string name, string content)
{
    headers_.push_back(make_pair(name, content));
    return 0;
}

string HTTPRequest::getHTTPHeader(string name)
{
    vector<pair<string, string>>::iterator iter;
    for(iter = headers_.begin(); iter != headers_.end(); iter++)
    {
        if((*iter).first == name)
        {
            return (*iter).second;
        }
    }
    return "There is no header name " + name;
}

int HTTPRequest::setHTTPHeaderVector(vector<pair<string, string>>* headervector)
{
    vector<pair<string, string>>::iterator iter;
    for(iter = headervector->begin(); iter != headervector->end(); iter++)
    {
        setHTTPHeader((*iter).first, (*iter).second);
    }
    return 0;
}

vector<pair<string, string>>* HTTPRequest::getHTTPHeaderVector()
{
    return &headers_;
}

int HTTPRequest::setRequestBody(string argbody)
{
    body_ = argbody;
    return 0;
}

string HTTPRequest::getRequestBody()
{
    return body_;
}

//set, get func end

int HTTPRequest::parseRequestPacket()
{
    size_t cursorbegin = 0, cursorend = 0;
    size_t headercursorbegin, headercursorend;
    string httpmethod, httpprotocol, header;
    string name, content;

    //http method
    httpmethod = updateCursor(cursorbegin, cursorend, " ", requestpacket_, 1);
    if(httpmethod == "GET")
    {
        method_ = GET;
    }
    else if(httpmethod == "PUT")
    {
        method_ = PUT;
    }
    else if(httpmethod == "POST")
    {
        method_ = POST;
    }
    else if(httpmethod == "PATCH")
    {
        method_ = PATCH;
    }
    else if(httpmethod == "DELETE")
    {
        method_ = DELETE;
    }
    else
    {
        method_ = NOT_IMPLEMENTED;
        return 0;
    }

    //url
    url_ = updateCursor(cursorbegin, cursorend, " ", requestpacket_, 1);

    //protocol
    httpprotocol = updateCursor(cursorbegin, cursorend, CRLF, requestpacket_, 1);
    if(httpprotocol == "HTTP/1.0")
    {
        protocol_ = HTTP1_0;
    }
    else if(httpprotocol == "HTTP/1.1")
    {
        protocol_ = HTTP1_1;
    }
    else
    {
        protocol_ = HTTP_UNSUPPORTED;
        return 0;
    }

    cursorbegin++; //CRLF

    //header parse start here
    while(1)
    {
        header = updateCursor(cursorbegin, cursorend, CRLF, requestpacket_, 1);
        //separate header line by line

        headercursorbegin = 0;
        headercursorend = 0;

        //name: content
        name = updateCursor(headercursorbegin, headercursorend, ":", header, 2);
        content = updateCursor(headercursorbegin, headercursorend, CRLF, header, 0);

        setHTTPHeader(name, content);
        cursorbegin++; //CRLF
        if(requestpacket_.substr(cursorbegin, 2) == CRLF) //one more CRLF
        {
            break;
        }
    }

    cursorbegin+=2;
    body_ = requestpacket_.substr(cursorbegin);

    return 0;
}

int HTTPRequest::makeRequest()
{
    string httpmethod, httpprotocol;

    switch(method_){
        case GET:
            httpmethod = "GET";
            break;
        case PUT:
            httpmethod = "PUT";
            break;
        case POST:
            httpmethod = "POST";
            break;
        default:
            return -1;
            break;
    }

    switch(protocol_){
        case HTTP1_0:
            httpprotocol = "HTTP/1.0";
            break;
        case HTTP1_1:
            httpprotocol = "HTTP/1.1";
            break;
        default:
            return -1;
            break;
    }

    requestpacket_ += httpmethod + " " + url_ + " " + httpprotocol + CRLF;
    vector<pair<string, string>>::iterator iter;
    for(iter = headers_.begin(); iter != headers_.end(); iter++){
        requestpacket_ += (*iter).first + ": " + (*iter).second + CRLF;
    }
    requestpacket_ += CRLF;
    requestpacket_ += body_;
    return 0;
}

size_t HTTPRequest::getRequestSize()
{
    return requestpacket_.length();
}

string* HTTPRequest::getRequestData()
{
    return &requestpacket_;
}

string HTTPRequest::updateCursor(size_t& cursorbegin, size_t& cursorend, string target, string obj, size_t next)
{
    string result;
    cursorend = obj.find_first_of(target, cursorbegin);
    result = obj.substr(cursorbegin, cursorend - cursorbegin);
    cursorbegin = cursorend + next;
    return result;
}

void HTTPRequest::resetData()
{
    requestpacket_ = "";
    headers_.clear();
}
