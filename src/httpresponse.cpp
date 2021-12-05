#include "httpresponse.h"

using namespace std;

HTTPResponse::HTTPResponse()
{
    body_ = "";
    responsepacket_ = "";
}

HTTPResponse::~HTTPResponse()
{
}

void HTTPResponse::addResponsePacket(char *buf, int& len)
{
    responsepacket_.append(buf, len);
}

int HTTPResponse::setProtocol(Protocol argprotocol)
{
    protocol_ = argprotocol;
    return 0;
}

Protocol HTTPResponse::getProtocol()
{
    return protocol_;
}

int HTTPResponse::setStatusCode(size_t argstatuscode)
{
    statuscode_ = argstatuscode;
    return 0;
}

size_t HTTPResponse::getStatusCode()
{
    return statuscode_;
}

int HTTPResponse::setReasonPhrase()
{
    switch(statuscode_)
    {
        case 200:
            reasonphrase_ = "OK";
            break;
        case 201:
            reasonphrase_ = "Created";
            break;
        case 400:
            reasonphrase_ = "Bad Request";
            break;
        case 403:
            reasonphrase_ = "Forbidden";
            break;
        case 404:
            reasonphrase_ = "Not Found";
            break;
        case 411:
            reasonphrase_ = "Length Required";
            break;
        case 500:
            reasonphrase_ = "Internal Server Error";
            break;
        case 501:
            reasonphrase_ = "Not Implemented";
            break;
        case 502:
            reasonphrase_ = "Bad Gateway";
            break;
        case 505:
            reasonphrase_ = "HTTP Version Not Supported";
            break;
        default:
            return -1;
            break;
    }

    return 0;
}

string HTTPResponse::getReasonPhrase()
{
    return reasonphrase_;
}

int HTTPResponse::setHTTPHeader(string name, string content)
{
    headers_.push_back(make_pair(name, content));
    return 0;
}

string HTTPResponse::getHTTPHeader(string name)
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

int HTTPResponse::setHTTPHeaderVector(vector<pair<string, string>>* headervector)
{
    vector<pair<string, string>>::iterator iter;
    for(iter = headervector->begin(); iter!=headervector->end(); iter++)
    {
        setHTTPHeader((*iter).first, (*iter).second);
    }
    return 0;
}

vector<pair<string, string>>* HTTPResponse::getHTTPHeaderVector()
{
    return &headers_;
}

int HTTPResponse::setResponseBody(string argbody)
{
    body_ = argbody;
    return 0;
}

string HTTPResponse::getResponseBody()
{
    return body_;
}

int HTTPResponse::makeResponse()
{
    string httpprotocol;
    string tmppacket;

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

    tmppacket += httpprotocol + " " + to_string(statuscode_) + " " + reasonphrase_ + CRLF;

    vector<pair<string, string>>::iterator iter;
    for(iter = headers_.begin(); iter != headers_.end(); iter++)
    {
        tmppacket += (*iter).first + ": " + (*iter).second + CRLF;
    }
    tmppacket += CRLF + body_;
    responsepacket_ = tmppacket;
    return 0;
}

int HTTPResponse::parseResponsePacket()
{
    size_t cursorbegin = 0, cursorend = 0;
    size_t headercursorbegin, headercursorend;
    string httpprotocol, httpstatuscode, header;
    string name, content;

    //protocol
    httpprotocol = updateCursor(cursorbegin, cursorend, " ", responsepacket_, 1);
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

    //status code
    httpstatuscode = updateCursor(cursorbegin, cursorend, " ", responsepacket_, 1);
    statuscode_ = atoi(httpstatuscode.c_str());

    //reason phrase
    reasonphrase_ = updateCursor(cursorbegin, cursorend, CRLF, responsepacket_, 1);

    cursorbegin++; //CRLF

    //header parse start here
    while(1)
    {
        header = updateCursor(cursorbegin, cursorend, CRLF, responsepacket_, 1);
        //separate header line by line

        headercursorbegin = 0;
        headercursorend = 0;

        //name: content
        name = updateCursor(headercursorbegin, headercursorend, ":", header, 2);
        content = updateCursor(headercursorbegin, headercursorend, CRLF, header, 0);

        setHTTPHeader(name, content);
        cursorbegin++; //CRLF
        if(responsepacket_.substr(cursorbegin, 2) == CRLF) //one more CRLF
        {
            break;
        }
    }

    cursorbegin+=2;
    body_ = responsepacket_.substr(cursorbegin);

    return 0;
}

size_t HTTPResponse::getResponseSize(void )
{
    return responsepacket_.length();
}

string* HTTPResponse::getResponseData()
{
    return &responsepacket_;
}

string HTTPResponse::updateCursor(size_t& cursorbegin, size_t& cursorend, string target, string obj, size_t next)
{
    string result;
    cursorend = obj.find_first_of(target, cursorbegin);
    result = obj.substr(cursorbegin, cursorend - cursorbegin);
    cursorbegin = cursorend + next;
    return result;
}
