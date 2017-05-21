/*------------------------------------------------------------------------------
 * test.cpp : for socket functions testing.
 * 
 * author   : Guangli Dong
 *
 * history  : 2016/07/07 new
 *
 *----------------------------------------------------------------------------*/

#include <conio.h>
#include "socket_lib.h"

int main(int argc, char *argv[])
{
    int sockfd;
    int recvnum, i;
    unsigned char buff[4096];
    FILE *output;
    const char ip[] = "192.168.0.2";
    int port = 8888;

    /* set for SiNan data stream */
    sockfd = creat_client_socket(ip, port);
    
    /* set output file */
    output = fopen("log.bin", "wb");

    if(0==sockfd) exit(0);
    
    while(!_kbhit()) {   // use keyboard hit action to stop loops
        memset(buff, 0x00, sizeof(buff));
        recvnum = recv(sockfd, (char *)buff, 4000, 0);

        if(SOCKET_ERROR==recvnum || 0==recvnum) {
            close_client_socket(sockfd);
            SLEEP(2000);
            printf("reconnecting...\n");
            sockfd = creat_client_socket(ip, port);
            continue;
        }
        for(i = 0; i<recvnum; i++) {
            fwrite(&(buff[i]), 1, 1, output);
#if 1
            if((i%16) == 0) printf("\n");
            printf("%02X ", buff[i]);
#endif
#if 0
            printf("%c", buff[i]);
#endif
        }
    }

    fclose(output);
    close_client_socket(sockfd);


    return 0;
}
