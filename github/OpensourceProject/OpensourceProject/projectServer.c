#include "socket_lib-master\socket_lib.h"
// #include <Shlwapi.h>
#include <time.h>
#include <errno.h>
#include <io.h> // access
#include <process.h> // _beginthreadex, mutex
#include <tchar.h>

// #pragma comment(lib, "Shlwapi.lib")

#define BUF_SIZE ((1025)*(512))

typedef struct _DeleteQueue {
	char path[MAX_PATH];
	struct _DeleteQueue *next;
} DeleteQueue;

HANDLE hMutex;
HANDLE hThread;
int run_queue_flag = 1, run_queue_count = 0;
DeleteQueue *queue_head;

void fileNameMaker(char *fName, char *url);
void __close(SOCKET sockfd, fd_set *readfds);
UINT WINAPI ThreadQueue(void *arg);

DWORD WINAPI ThreadQueue(void *arg)
{
	DeleteQueue *queue_tmp;
	DeleteQueue *queue_del;
	while (run_queue_flag) {
		if (!run_queue_count) {
			SuspendThread(hThread);
		}

		WaitForSingleObject(hMutex, INFINITE);
		queue_tmp = queue_head->next;
		while (queue_tmp != NULL) {
			if (remove(queue_tmp->path) != 0) {
				continue;
			}
			else {
				run_queue_count--;
				queue_del = queue_tmp;
				queue_tmp = queue_tmp->next;
				queue_head->next = queue_tmp;
				free(queue_del);
			}
		}
		ReleaseMutex(hMutex);
	}
	_endthreadex(0);
	return 0;
}

void __close(SOCKET sockfd, fd_set *readfds)
{
	closesocket(sockfd);
	FD_CLR(sockfd, readfds);
}

int main(int argc, char **argv)
{
	FILE* fp;
	char fName[MAX_PATH];
	char url[MAX_PATH];
	char buf[BUF_SIZE];

	int readn, BigE;
	int i = 0;

	fd_set readfds, allfds;

	const char ip[] = "127.0.0.1";
	const int port = 2222;

	int fd_num, maxfd = 0;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN server_addr, client_addr;
	int addrlen;

	DWORD dwThreadID;

	hMutex = CreateMutex(
		NULL,	// 디폴트 보안 관리자
		FALSE,	// 누구나 소유할 수 있는 상태로 생성
		NULL	// numaned mutex
	);

	if (hMutex == NULL) {
		_tprintf(TEXT("CreateMutex error: %d\n"), GetLastError());
		return 0;
	}
	queue_head = (DeleteQueue *)malloc(sizeof(DeleteQueue));
	queue_head->path[0] = 0x00;
	queue_head->next = 0;
	hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadQueue, NULL, 0, &dwThreadID);

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
			printf("connected client: %d [%s:%d]\n", hClntSock, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			continue;
		}

		for (i = 0; i <= allfds.fd_count; i++) // maxfd; i++)
		{
			if (allfds.fd_array[i] == hServSock) continue;

			hClntSock = allfds.fd_array[i];
			if (FD_ISSET(hClntSock, &allfds))
			{
				int fileSize = 0, totalSize = 0;
				fileNameMaker(fName, url);

				printf("[fd: %u] Create File: %s\n", hClntSock, fName);

				if ((fp = fopen(fName, "wb")) == NULL) {
					puts("fopen() error!");
					__close(hClntSock, &readfds);
					break;
				}

				readn = recv(hClntSock, (char *)&BigE, sizeof(int), 0);
				fileSize = ntohl(BigE);
				while (1) {
					readn = recv(hClntSock, buf, BUF_SIZE - 1, 0);
					totalSize += readn;
					// printf("[fd:%u] recv %u/%u bytes\n", hClntSock, totalSize, fileSize);
					if (readn == 0) // 상대 소켓 종료
					{
						printf("[fd:%u] file receive end!\n", hClntSock);
						fclose(fp);
						__close(hClntSock, &readfds);
						break;
					}
					else if (totalSize >= fileSize) {
						int len = htonl(strlen(url));
						send(hClntSock, (char*)&len, sizeof(int), 0);
						//printf("url 전송...: %s(%d)\n", url, len);
						send(hClntSock, url, MAX_PATH, 0);
						// send(hClntSock, url, len, 0);
						printf("url 전송완료!\n");
					}
					else if (readn == SOCKET_ERROR)
					{
						DeleteQueue *queueTmp = (DeleteQueue*)malloc(sizeof(DeleteQueue));
						printf("[fd:%u] file receive error!\n", hClntSock);
						fclose(fp);
						__close(hClntSock, &readfds);
						if (queueTmp == NULL) {
							printf("[fd: %u] file memory allocation error!\n", hClntSock, fName);
						}
						else {
							WaitForSingleObject(hMutex, INFINITE);
							strncpy_s(queueTmp->path, MAX_PATH, fName, MAX_PATH);
							queueTmp->next = NULL;
							queue_head->next = queueTmp;
							run_queue_count++;
							ReleaseMutex(hMutex);
							puts("thread resume!");
							ResumeThread(hThread);
						}
						break;
					}
					fwrite(buf, sizeof(char), readn, fp);
					// printf("[fd:%u] recv %u bytes\n", hClntSock, readn);
				}

				if (--fd_num <= 0)
					break;
			}
		}
	}

	run_queue_flag = 0;
	WaitForSingleObject(hThread, INFINITE);
	close_client_socket(hServSock);
	CloseHandle(hThread);
	free(queue_head);
	return 0;
}

void fileNameMaker(char *fName, char *url) {
	time_t timer;
	struct tm *curTime;

	char strFolderPath[] = "C:\\Tomcat 7.0\\webapps\\ROOT\\download";
	int nResult, idx = 0;

	timer = time(NULL);
	curTime = localtime(&timer);

	nResult = mkdir(strFolderPath);

	memset(url, 0, sizeof(MAX_PATH));
	sprintf(url, "http://127.0.0.1/download/%04d%02d%02d_%02d%02d%02d.png",
		curTime->tm_year + 1900, curTime->tm_mon + 1, curTime->tm_mday,
		curTime->tm_hour, curTime->tm_min, curTime->tm_sec
	);
	sprintf(fName, "%s\\%04d%02d%02d_%02d%02d%02d.png", strFolderPath,
		curTime->tm_year + 1900, curTime->tm_mon + 1, curTime->tm_mday,
		curTime->tm_hour, curTime->tm_min, curTime->tm_sec
	);

	while (1) {
		//if (PathFileExists(fName) == TRUE) {
		//	break;
		//}
		if (access(fName, 0) == -1) {
			break;
		}

		memset(url, 0, sizeof(MAX_PATH));
		sprintf(fName, "%s\\%04d%02d%02d_%02d%02d%02d(%d).png", strFolderPath,
			curTime->tm_year + 1900, curTime->tm_mon + 1, curTime->tm_mday,
			curTime->tm_hour, curTime->tm_min, curTime->tm_sec, idx
		);
		sprintf(url, "http://127.0.0.1/download/%04d%02d%02d_%02d%02d%02d(%d).png",
			curTime->tm_year + 1900, curTime->tm_mon + 1, curTime->tm_mday,
			curTime->tm_hour, curTime->tm_min, curTime->tm_sec, idx++
		);
	}
}