/*

*/

#ifndef CLIENTSOCK__h
#define CLIENTSOCK__h

#include "Client.h"

/*class MyClient : public Client
{
private:
    int sockfd;

public:
    MyClient(void);
    ~MyClient(void);
    virtual int connect(IPAddress_t ip, uint16_t port);
    virtual int connect(const char *host, uint16_t port);
    virtual uint8_t connected();
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buf, size_t size);
    virtual int available();
    virtual int read();
    virtual int read(uint8_t *buf, size_t size);
    virtual int peek();
    virtual void flush();
    virtual void stop();
    virtual operator bool();
};*/
void    clientSocket_init       (void);
void    clientSocket_destroy    (void);

extern const Client_t clientSock;

#endif /* CLIENTSOCK__h */
