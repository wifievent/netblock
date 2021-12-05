#pragma once

typedef enum {GET, PUT, HEAD, POST, NOT_IMPLEMENTED} Method;
//we can add more
typedef enum {HTTP1_0, HTTP1_1, HTTP_UNSUPPORTED} Protocol;
//we can add more

#define	CR  '\r'
#define LF  '\n'
#define	CRLF    "\r\n"

#include <string>
#include <vector>
