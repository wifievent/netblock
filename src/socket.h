#ifndef SOCKET_H
#define SOCKET_H

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>

#define BUFSIZE 65536

class Socket
{
public:
    Socket();
    virtual ~Socket() {}

    virtual int disconnect() = 0;
    virtual int send(char* buf, size_t len) = 0;
    virtual int recv(char* buf, size_t len) = 0;
};

#endif // SOCKET_H
