/**
 * 高并发服务器-多线程版本
 * stanley
 * 2018-03-28
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>

#include "wrap.h"

//客户端线程最大值
#define MAX_CLT 256	
#define MAXSIZE 8192
#define SERV_PORT 8000
#define INET_ADDRSTRLEN 16

//客户端信息封装
struct SClient_info {
	struct sockaddr_in addr_clie;	//客户端地址信息
	int fd_connect;	//客户端文件描述符
};

//回调函数 子进程处理逻辑
void* child_work(void *arg);

static int count = 0;	//当前线程数量

int
main(
		void
	)
{
	int fd_listen = 0;
	int fd_connect = 0;
	struct sockaddr_in addr_serv;
	struct sockaddr_in addr_clie;
	socklen_t addr_clie_len = 0;
	pthread_t pthid;
	struct SClient_info cInfo[MAX_CLT];

	fd_listen = Socket(AF_INET, SOCK_STREAM, 0);

	//初始化地址
	bzero(&addr_serv, sizeof(addr_serv));
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port = htons(SERV_PORT);
	addr_serv.sin_addr.s_addr = htons(INADDR_ANY);

	Bind(fd_listen, (struct sockaddr*)&addr_serv, sizeof(addr_serv));

	Listen(fd_listen, 128);

	printf("Server established...Accepting connections...\n");
	//线程处理连接客户端
	while(1) {
		printf("----当前客户端数量  %d\n", count);
		addr_clie_len = sizeof(addr_clie);
		fd_connect = Accept(fd_listen, (struct sockaddr*)&addr_clie, &addr_clie_len);

		cInfo[count].addr_clie = addr_clie;
		cInfo[count].fd_connect = fd_connect;

		/*线程最大时，处理*/
		pthread_create(&pthid, NULL, child_work, (void *)&cInfo[count]);
		pthread_detach(pthid);

		count++;
	}

	Close(fd_listen);

	return 0;
}

//回调函数 子进程处理逻辑
void*
child_work(
		void *arg	//struct SClient_info
		)
{
	struct SClient_info *cInfo = (struct SClient_info*)arg;
	ssize_t nRead = 0;
	char buf[MAXSIZE];
	char str[INET_ADDRSTRLEN];
	int i = 0;

	while(1) {
		nRead = Read(cInfo->fd_connect, buf, MAXSIZE);
		if(0 == nRead) {
			printf("The client %s:%d closed...\n",
				inet_ntop(AF_INET, &(cInfo->addr_clie.sin_addr), str, sizeof(str)),
				ntohs(cInfo->addr_clie.sin_port));
			break;
		}
		printf("Received from %s at PORT %d\n",
				inet_ntop(AF_INET, &(cInfo->addr_clie.sin_addr), str, sizeof(str)),
				ntohs(cInfo->addr_clie.sin_port));
		for(i = 0; i < nRead; ++i) {
			buf[i] = toupper(buf[i]);
		}

		Write(STDOUT_FILENO, buf, nRead);
		Write(cInfo->fd_connect, buf, nRead);
	}

	Close(cInfo->fd_connect);

	return (void *)0;
}
