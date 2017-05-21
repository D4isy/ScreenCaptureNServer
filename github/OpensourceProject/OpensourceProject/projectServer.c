#include "socket_lib-master\socket_lib.h"

#define BUF_SIZE 100

int main(void) {

	const char ip[] = "127.0.0.1";
	const int port = 2222;

	socket_t hServSock, hClntSock;
	SOCKADDR_IN clntAddr;

	TIMEVAL timeout;
	fd_set reads, cpyReads;

	int adrSz;
	int hListener;

	// for test 
	char buf[BUF_SIZE];
	FILE *fp;

	hServSock = creat_server_socket(ip, port);
	
	FD_ZERO(&reads);
	FD_SET(hServSock, &reads);

	while (1) {
		cpyReads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		if ((hListener = select(0, &cpyReads, 0, 0, &timeout)) == SOCKET_ERROR) {
			printf("select() Error!\n");
			break;
		}
		else {
			for (unsigned int i = 0; i < reads.fd_count ; i++) {
				if (FD_ISSET(reads.fd_array[i], &cpyReads)) {
					if (reads.fd_array[i] == hServSock) {		// 연결 요청을 받은 경우 
						adrSz = sizeof(clntAddr);
						hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &adrSz);
						FD_SET(hClntSock, &reads);
						printf("connected client: %d \n", hClntSock);
					}
					else {			// client로 부터 데이터가 전송된 경우 
						int readCnt;
						fp = fopen("recvFile.txt", "w+");

						while (1) {
							readCnt = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0); 
							if (readCnt < BUF_SIZE - 1) { 
								if (feof(fp) != 0) { 
									fwrite(buf, sizeof(char), readCnt, fp);
								} 
								else { 
									puts("file send() error!\n");
								}
								break;
							}
							fwrite(buf, sizeof(char), BUF_SIZE - 1, fp);
							fclose(fp);
						} 
					}
				}
			}
		}
	}

}