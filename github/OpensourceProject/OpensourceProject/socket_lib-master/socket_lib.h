/*------------------------------------------------------------------------------
 * socket_lib.h
 * 
 * author   : Guangli Dong
 *
 * history  : 2016/07/07 new
 *
 *----------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------/
 | usage - server:
 |
 | 1. creat server socket
 |      socket_t servfd = creat_server_socket("192.168.3.212", 40001);
 |
 | 2. accept request from client using standard socket fuction
 |      socket_t clntfd = accept(servfd, (struct sockaddr*)NULL, NULL);
 |
 | 3. send and recv operations using standard socket function
 |      n = send(sockfd, sendline, stren(sendline), 0);
 |      n = recv(clntfd, recvline, RECVSIZE, 0);
 |
 | 4. close connected client socket
 |      close_client_socket(clntfd);
 |
 | 5. close server socket
 |      close_server_socket(servfd);
 |
 |
 | usage - client:
 |
 | 1. creat client socket
 |      socket_t clntfd = creat_client_socket("192.168.3.212", 40001);
 |
 | 2. send and recv operations using standard socket function
 |      n = send(clntfd, sendline, stren(sendline), 0);
 |      n = recv(clntfd, recvline, RECVSIZE, 0);
 |
 | 3. close client socket
 |      closesocket(clntfd);
 | note:
 |  If the connection is interrupt, recv() will return SOCKET_ERROR or 0, then 
 |  you can reconnect tcp server by the following steps:
 |  1. close the original socket hander
 |      closesocket(clntffd);
 |  2. sleep a while
 |      SLEEP(2000);
 |  3. re-open the socket hander
 |      clntfd = creat_client_socket("192.168.3.212", 40001);
 |
 *----------------------------------------------------------------------------*/

#ifndef SOCKET_LIB_H
#define SOCKET_LIB_H

/* includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <WinSock2.h>
#pragma comment (lib, "ws2_32.lib")
#define SLEEP(a) Sleep(a)


/* macros --------------------------------------------------------------------*/
#define socket_t    SOCKET

#ifdef __cplusplus
extern "C" {
#endif

/* extern functions ----------------------------------------------------------*/
extern socket_t creat_server_socket(const char *IP, int PORT);
extern socket_t creat_client_socket(const char *IP, int PORT);
extern void     close_server_socket(socket_t sock);
extern void     close_client_socket(socket_t sock);

#ifdef __cplusplus
}
#endif

#endif // SOCKET_LIB_H
