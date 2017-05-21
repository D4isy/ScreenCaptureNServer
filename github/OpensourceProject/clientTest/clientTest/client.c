#include "socket_lib-master\socket_lib.h"

#define BUF_SIZE ((1024)*(512))

int main(void)
{
	int i=0;
	while (++i)
	{
		socket_t servSock;

		FILE* fp;
		//char fName[40];
		char fBuffer[BUF_SIZE];
		int readSize, readTotalSize, fileSize;

		const char IP[] = "127.0.0.1";
		int port = 2222;

		printf("i: %d\n", i);
		servSock = creat_client_socket(IP, port);
		if (servSock == SOCKET_ERROR) { return; }

		//	printf("전송할 파일 이름 입력(확장자까지): ");
		//	gets_s(fName, sizeof(fName));

		printf("connect %u\n", servSock);
		fp = fopen("20170511_213040.png", "rb");
		if (fp == NULL) {
			puts("fopen() error!");
		}

		// file의 크기 계산 
		fseek(fp, 0, SEEK_END);
		fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		readTotalSize = 0;

		// 받아온 data의 크기가 file의 크기와 같거나 클 때 까지 파일의 내용을 받아옴 
		while (fileSize > readTotalSize) {
			memset(fBuffer, 0x00, BUF_SIZE);
			readSize = fread(fBuffer, sizeof(char), BUF_SIZE, fp);
			if (send(servSock, fBuffer, readSize, 0) == SOCKET_ERROR) {
				break;
			}
			printf("send %u/%u bytes\n", readSize, fileSize);
			readTotalSize = readTotalSize + readSize;
		}

		fclose(fp);
		closesocket(servSock);
	}

	return 0;
}