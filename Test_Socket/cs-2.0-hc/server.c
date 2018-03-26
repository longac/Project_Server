#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>

#include "wrap.h"

#define SERV_PORT 8888
#define MAXLEN 8192

/*
 * 高并发服务器-多进程版本
 * stanley
 * 2018-03-26
 */
int
main(
		void
	)
{
	int lfd = 0;	//监听端文件描述符
	int cfd = 0;	//客户端文件描述符
	struct sockaddr_in serv_addr;	//服务器地址
	struct sockaddr_in clie_addr;	//客户端连接地址
	socklen_t clie_addr_len = 0;	//客户端地址结构体长度
	char clie_buff[BUFSIZ];			//客户端IP “127.0.0.1”
	int i = 0;
	int n = 0;	//服务器收到的数据长度
	char buf[MAXLEN];	//读取数据缓冲区

	pid_t pid = 0;

	//创建套接字
	lfd = Socket(AF_INET, SOCK_STREAM, 0);

	//服务器地址初始化
	serv_addr.sin_family = AF_INET;	//协议
	serv_addr.sin_port = htons(SERV_PORT);	//端口号
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);	//IP地址

	//绑定地址
	Bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	//设置监听上线
	Listen(lfd, 128);

	printf("Server established...Accepting connections...\n");
	while(1) {
		clie_addr_len = sizeof(clie_addr);
		cfd = Accept(lfd, (struct sockaddr*)&clie_addr, &clie_addr_len);
		//给每一个连上的客户端创建进程处理逻辑
		pid = fork();
		if(0 == pid) {	//子进程主要负责处理客户端的业务请求
			printf("A client connect! IP : %s  PORT : %d\n",
					inet_ntop(AF_INET, &clie_addr.sin_addr, clie_buff, sizeof(clie_buff)),
					ntohs(clie_addr.sin_port));
			Close(lfd);


			//处理客户端发来的数据
			while(1) {
				n = Read(cfd, buf, MAXLEN);
				if(0 == n) {
					printf("The client side has been closed.\n");
					break;
				}

				printf("Recevied from %s:%d\n",
						inet_ntop(AF_INET, &clie_addr.sin_addr, clie_buff, sizeof(clie_buff)),
						ntohs(clie_addr.sin_port));
				//数据转换处理
				for(i = 0; i < n; ++i) {
					buf[i] = toupper(buf[i]);
				}

				//终端显示
				Write(STDOUT_FILENO, buf, n);
				//回显客户端
				Write(cfd, buf, n);
			}

			Close(cfd);
			return 0;
		}
		else if(pid > 0){	//主进程主要负责回收子进程资源
			Close(cfd);
		}
		else {
			perror("fork error: ");
			exit(-1);
		}
	}
	return 0;
}
