/*

*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WINDOWS

#ifdef WINDOWS
  #include <winsock2.h>
  #include <Windows.h>
  #include <ws2tcpip.h>
    //  #pragma comment(lib,"ws2_32.lib") //Winsock Library
  #define socklen_t int
  #define errno WSAGetLastError()
  #define ioctl ioctlsocket
#else
  #include <arpa/inet.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netdb.h>
  #include <sys/ioctl.h>
#endif

#include "clientSock.h"

/******************************************************************************
 * Private Function Prototypes
 *****************************************************************************/
int     clientSocket_connectIP  (IPAddress_t ip, uint16_t port);
int     clientSocket_connectHost(const char *host, uint16_t port);
uint8_t clientSocket_connected  ();
size_t  clientSocket_write      (uint8_t);
size_t  clientSocket_writeMulti (const uint8_t *buf, size_t size);
int     clientSocket_available  ();
int     clientSocket_read       ();
int     clientSocket_readMulti  (uint8_t *buf, size_t size);
int     clientSocket_peek       ();
void    clientSocket_flush      ();
void    clientSocket_stop       ();

/******************************************************************************
 * Private Variable
 *****************************************************************************/
const Client_t clientSock =
{
    &clientSocket_connectIP,
    &clientSocket_connectHost,
    &clientSocket_connected,
    &clientSocket_write,
    &clientSocket_writeMulti,
    &clientSocket_available,
    &clientSocket_read,
    &clientSocket_readMulti,
    &clientSocket_peek,
    &clientSocket_flush,
    &clientSocket_stop
};

static int clientSockfd;


/******************************************************************************
 * Private Functions
 *****************************************************************************/
static void error(const char *format, ...)
//static void error(const char *msg)
{
    va_list args;
    va_start(args, format);
    const int size = 100;
    char buf[size] = {0};

    snprintf(buf, size, format, args);
    perror(buf);
    printf("errno: %d\r\n", errno);

    va_end(args);
//    exit(0);
}

static void log(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

/******************************************************************************
 * Function implementations
 *****************************************************************************/
//MyClient::MyClient(void)
void clientSocket_init(void)
{
#ifdef WINDOWS
    WSADATA wsaData;

    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0)
    {
        log("WSAStartup failed: %d\n", iResult);
        while(1)
        {
            error("error starting ethernet interface!");
        }
    }
#endif
    clientSockfd = -1;
}

/******************************************************************************
 * Function implementations
 *****************************************************************************/

//MyClient::~MyClient(void)
void clientSocket_destroy(void)
{
    log("DESCTRUCTOR, close socket: %d\n", clientSockfd);
    clientSocket_stop();
}


//int MyClient::connect(IPAddress_t ip, uint16_t port)
int clientSocket_connectIP(IPAddress_t ip, uint16_t port)
{
    return 0;
}

//int MyClient::connect(const char *host, uint16_t port)
int clientSocket_connectHost(const char *host, uint16_t port)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;

    log("\r\nsockfd:%d", clientSockfd);
    clientSockfd = socket(AF_INET, SOCK_STREAM, 0);
    log("\r\nsockfd:%d", clientSockfd);

    if (clientSockfd < 0)
    {
        error("ERROR opening socket");
    }
    else
    {
        if (host==NULL)
        {
            error("ERROR invalid hostname");
        }
        else
        {
            server = gethostbyname(host);
            if (server == NULL)
            {
                error("ERROR, no such host\n");
            }
            else
            {
                memset((char *)&serv_addr, 0, sizeof(serv_addr));
                memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
                serv_addr.sin_family = AF_INET;
                serv_addr.sin_port = htons(port);

                log("\n  Connecting to host: %u.%u.%u.%u:%d, sockfd: %d ... ",
                        server->h_addr[0]&0xFF,
                        server->h_addr[1]&0xFF,
                        server->h_addr[2]&0xFF,
                        server->h_addr[3]&0xFF,
                                port, clientSockfd);

                if (0 != ::connect(clientSockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)))
                {
                    error("ERROR connecting");
                }
                else
                {
                    log("  [connected]\r\n");
                    struct timeval tv;
                    tv.tv_sec = 5;  /* 30 Secs Timeout */
                    tv.tv_usec = 30;  // Not init'ing this can cause strange errors
                    setsockopt(clientSockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

                    return (clientSockfd>=0);
                }
            }
        }
    }
    clientSocket_stop();
    return -1;
}

//uint8_t MyClient::connected()
uint8_t clientSocket_connected()
{
    return (clientSockfd >= 0);
}

//size_t MyClient::write(uint8_t)
size_t clientSocket_write(uint8_t)
{
    log("\r\nMyClient::write\r\n");
    return 0;
}

//size_t MyClient::write(const uint8_t *buf, size_t size)
size_t clientSocket_writeMulti(const uint8_t *buf, size_t size)
{
    int n = -1;
    log("  sending message to server[%d]...", clientSockfd);
    if(clientSockfd<0)
    {
        log("\nERROR Sending to invalid socket: %d", clientSockfd);
    }
    else
    {
        n = send(clientSockfd, (char*)buf, size, 0);
        if (0 > n )
        {
            //log("\nERROR (%d) writing to socket: %d\n", errno, handle);
            error("Sock_Send: write failed");
        }
        else
        {
            uint8_t idx = 0;
            log("\r\n  buf: size: %d,  n: %d", size, n);
            log("\r\n  [MSGOUT HEX]  :");
            while(idx<n)
            {
                log("%d ",buf[idx]); 
                idx++;
            }
            log("<<:");

            idx = 0;
            log("\r\n  [MSGOUT ASCII]:");
            while(idx<n)
            {
                log("%c",buf[idx]); 
                idx++;
            }
            log("<<:\r\n");
        }
    }
    return n;
}

//int MyClient::available()
int clientSocket_available()
{
//    log("\r\nMyClient::available::");

/*    fd_set rfds;
    int retval;
    struct timeval tv;
    tv.tv_sec  = 1;
    tv.tv_usec = 1;

    FD_ZERO(&rfds);
    FD_SET(clientSockfd, &rfds);
    retval = select(1, &rfds, NULL, NULL, &tv);

    if (retval == -1)
    {
//        log("no\r\n");
        return 0;
    }
//    log("yes\r\n");
    return 1;*/

    int count;
    ioctlsocket(clientSockfd,FIONREAD,(u_long*)&count);
    //ioctl(clientSockfd, FIONREAD, &count);
//    log("\r\nMyClient::available::%d\r\n", count);
    return count;
}

//int MyClient::read()
int clientSocket_read()
{
    char buf;
    int length;
//    log("\r\nMyClient::read(1): ");
    length = recv(clientSockfd, (char*)&buf, 1, 0/*flags*/);
    if(length>0)
    {
//        log("length:%d:::0x%02x, %d\r\n",length, buf&0xFF, buf&0xFF);
        return buf&0xFF;
    }
//    log("nothing\r\n");
    return 0;
}

//int MyClient::read(uint8_t *buf, size_t size)
int clientSocket_readMulti(uint8_t *buf, size_t size)
{
    log("\r\nMyClient::read(2)\r\n");
    return 0;
}

//int MyClient::peek()
int clientSocket_peek()
{
    log("\r\nMyClient::peek\r\n");
    return 0;
}

//void MyClient::flush()
void clientSocket_flush()
{
    log("\r\nMyClient::flush\r\n");
}

//void MyClient::stop()
void clientSocket_stop()
{
    log("\r\nMyClient::stop\r\n");
    log("  closing socket[%d]... ", clientSockfd);
    if(::close(clientSockfd) != 0)
    {
        error("Failed");
    }
    else
    {
        log("ok\r\n");

    }
    clientSockfd = -1;
}
