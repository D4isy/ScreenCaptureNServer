/*------------------------------------------------------------------------------
 * socket_lib.c
 * 
 * author   : Guangli Dong
 *
 * history  : 2016/07/07 new
 *            2017/03/07 change error return code from -1 to 0
 *            2017/03/15 set recv timeout quit code.
 * note     :
 *            For win32, socket_t is SOCKET type, in fact, it is unsigned int,
 *            so we can't assign a nagative value to socket_t, ie, any error 
 *            code should be set to 0.
 *
 *----------------------------------------------------------------------------*/

/* includes ------------------------------------------------------------------*/
#include "socket_lib.h"

extern socket_t creat_server_socket(const char *IP, int PORT)
{
    /* local variables */
    socket_t sock;
    struct sockaddr_in servaddr;
    WORD    wVersionRequested;
    WSADATA wsaData;
    int err, opt, ret;
    
    /* 1. setup socket lib version */
    wVersionRequested = MAKEWORD(2, 0);
    err = WSAStartup(wVersionRequested, &wsaData);
    if(err != 0)
    {
        printf("Socket2.0 initialise failed, exit!\n");
        return 0;
    }
    if( LOBYTE(wsaData.wVersion) != 2 || HIBYTE( wsaData.wVersion) != 0)
    {
        printf("Socket version error, exit !\n");
        WSACleanup();
        return 0;
    }

    /* 2. creat socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET)
    {
        printf("Creat socket failed, exit!\n");
        return 0;
    }

    /* 3. enable address reuse */
    opt = 1;
    ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, 
        sizeof(opt));

    /* 4. bind socket with local address */    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP);
    servaddr.sin_port = htons(PORT);
    bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr));

    /* 5. set sock to listen mode */
    if( listen(sock, 10) == SOCKET_ERROR)
    {
        printf("Socket listen error, exit !\n");
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    /* 6. return socket */
    return sock;
}

/* creat_client_socket() for windows
 * returns:
 *  0          -> error
 * positive    -> ok
 */
extern socket_t creat_client_socket(const char *IP, int PORT)
{
    /* local variables */
    socket_t sock;
    struct sockaddr_in servaddr;
    WORD    wVersionRequested;
    WSADATA wsaData;
    int err, ret;
    int timeout = 3000; //3s


    /* 1. setup socket lib version */
    wVersionRequested = MAKEWORD(2, 0);
    err = WSAStartup(wVersionRequested, &wsaData);
    if(err != 0)
    {
        printf("Socket2.0 initialise failed, exit!\n");
        return 0;
    }
    if( LOBYTE(wsaData.wVersion) != 2 || HIBYTE( wsaData.wVersion) != 0)
    {
        printf("Socket version error, exit !\n");
        WSACleanup();
        return 0;
    }

    /* 2. creat socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET)
    {
        printf("Creat socket failed, exit!\n");
        return 0;
    }

    /* 3. connet socket with server address */    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP);
    servaddr.sin_port = htons(PORT);

    /* set recv timeout quit */
    ret=setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));

    if( connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) 
        == SOCKET_ERROR)
    {
        printf("Connect server error!\n");
        closesocket(sock);
        WSACleanup();
        return 0;
    }
    
    /* 4. return socket */
    return sock;
}

extern void close_server_socket(socket_t sock)
{
    closesocket(sock);
    WSACleanup();
}

extern void close_client_socket(socket_t sock)
{
    close_server_socket(sock);
}
