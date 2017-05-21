#include "socket_lib-master\socket_lib.h"

#include <time.h>
#include <tchar.h>

#define BUF_SIZE 10 //((1025)*(512))

void __close(SOCKET sockfd, fd_set *readfds)
{
	closesocket(sockfd);
	FD_CLR(sockfd, readfds);
}

int main(int argc, char **argv)
{
	FILE* fp;
	char fName[MAX_PATH];
	char buf[BUF_SIZE];

	int readn;
	int i = 0;

	fd_set readfds, allfds;

	const char ip[] = "127.0.0.1";
	const int port = 2222;

	int fd_num, maxfd = 0;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN server_addr, client_addr;
	int addrlen;

	FD_ZERO(&readfds);
	hServSock = creat_server_socket(ip, port);

	FD_SET(hServSock, &readfds);

	maxfd = hServSock;
	while (1)
	{
		allfds = readfds;
		// printf("Select Wait %d\n", maxfd);

		fd_num = select(maxfd + 1, &allfds, (fd_set *)0,
			(fd_set *)0, NULL);

		if (fd_num == SOCKET_ERROR) {
			printf("select error!\n");
			break;
		}

		if (FD_ISSET(hServSock, &allfds))
		{
			addrlen = sizeof(client_addr);
			hClntSock = accept(hServSock,
				(struct sockaddr *)&client_addr, &addrlen);

			FD_SET(hClntSock, &readfds);

			if (hClntSock > maxfd)
				maxfd = hClntSock;
			// printf("connected client: %d \n", hClntSock);
			continue;
		}

		for (i = 0; i <= allfds.fd_count; i++) // maxfd; i++)
		{
			if (allfds.fd_array[i] == hServSock) continue;

			hClntSock = allfds.fd_array[i];
			if (FD_ISSET(hClntSock, &allfds))
			{
				fileNameMaker(fName);

				printf("[fd: %u] Create File: %s\n", hClntSock, fName);

				if ((fp = fopen(fName, "wb")) == NULL) {
					puts("fopen() error!");
					__close(hClntSock, &readfds);
					break;
				}

				while (1) {
					readn = recv(hClntSock, buf, BUF_SIZE - 1, 0);
					if (readn == 0)
					{
						// printf("[fd:%u] file receive end!\n", hClntSock);
						fclose(fp);
						__close(hClntSock, &readfds);
						break;
					}
					else if (readn == SOCKET_ERROR)
					{
						printf("[fd:%u] file receive error!\n", hClntSock);
						fclose(fp);
						__close(hClntSock, &readfds);
						break;
					}
					fwrite(buf, sizeof(char), BUF_SIZE - 1, fp);
					// printf("[fd:%u] recv %u bytes\n", hClntSock, readn);
				}

				if (--fd_num <= 0)
					break;
			}
		}
	}

	close_client_socket(hServSock);
	return 0;
}

void fileNameMaker(char *fName) {
   time_t timer;
   struct tm *curTime;

   char strFolderPath[] = "C:\\server";
   int nResult, idx=0;

   timer = time(NULL);
   curTime = localtime(&timer);
	
   nResult = mkdir(strFolderPath);
	
   sprintf(fName, "%s\\%04d%02d%02d_%02d%02d%02d.png", strFolderPath,
      curTime->tm_year + 1900, curTime->tm_mon + 1, curTime->tm_mday,
      curTime->tm_hour, curTime->tm_min, curTime->tm_sec
   );

   while (1) {
      //if (PathFileExists(fName) == TRUE) {
      //   break;
      //}
      if (access(fName, 0) == -1) {
         break;
      }

      sprintf(fName, "%s\\%04d%02d%02d_%02d%02d%02d(%d).png", strFolderPath,
         curTime->tm_year + 1900, curTime->tm_mon + 1, curTime->tm_mday,
         curTime->tm_hour, curTime->tm_min, curTime->tm_sec, idx++
         );
   }
}
