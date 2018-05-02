#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include "wrap.h"
#include "server_select.h"

void
server_select(unsigned short port)
{
	int fd_listen = 0;	//监听
	int fd_connect = 0;	//客户端连接
	int fd_socket = 0;	//处理
	int fd_max = 0;		//当前使用的最大文件描述符

	struct sockaddr_in addr_serv;
	struct sockaddr_in addr_clie;

	fd_set allset;		//所有连接文件描述符位图
	fd_set rset;		//读请求文件描述符位图

	int clients[FD_SETSIZE];	//本地文件描述符数组
	int pos = 0;				//下标

	//socket();
	fd_listen = Socket(AF_INET, SOCK_STREAM, 0);

	//setsockopt()	端口复用
	int opt = 0;
	setsockopt(fd_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	//bind();
	memset(&addr_serv, 0, sizeof(addr_serv));
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port = htons(port);
	addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
	Bind(fd_listen, (struct sockaddr*)&addr_serv, sizeof(addr_serv));

	//listen();
	Listen(fd_listen, 128);

	//初始化
	fd_max = fd_listen;
	pos = -1;
	memset(clients, -1, sizeof(clients));

	//select集合
	FD_ZERO(&allset);
	FD_SET(fd_listen, &allset);

	int nReady = 0;			//客户端读请求个数
	socklen_t addr_clie_len = 0;	//客户端长度
	char str[INET_ADDRSTRLEN];	//客户端地址字符串
	int i = 0;				//遍历
	char buf[BUFSIZ];
	//select逻辑
	while(1) {
		rset = allset;		//更新读时间文件集合
		nReady = select(fd_max + 1, &rset, NULL, NULL, NULL);
		if(nReady < 0) {
			perror("select error: ");
			break;
		}

		//先处理读请求
		if(FD_ISSET(fd_listen, &rset)) {
			addr_clie_len = sizeof(addr_clie);
			//accept()
			fd_connect = Accept(fd_listen, (struct sockaddr*)&addr_clie, &addr_clie_len);
			printf("[%s:%d] is connected\n",
					inet_ntop(AF_INET, &addr_clie.sin_addr, str, sizeof(str)),
					ntohs(addr_clie.sin_port));

			//寻找本地文件描述符中没有被使用的
			for(i = 0; i < FD_SETSIZE; ++i) {
				if(clients[i] < 0) {	//-1
					clients[i] = fd_connect;
					break;
				}
			}

			if(i == FD_SETSIZE) {
				fputs("too many clients\n", stderr);
				printf("please waiting for...\n");
			}

			FD_SET(fd_connect, &allset);

			//更新当前最大文件描述符
			if(fd_connect > fd_max) {
				fd_max = fd_connect;
			}

			if(i > pos) {
				pos = i;
			}

			//所有监听事件处理完毕
			if(0 == (--nReady)) {
				continue;
			}
		}

		//处理读事件
		for(i = 0; i <= pos; ++i) {
			if((fd_socket = clients[i]) < 0) {
				continue;
			}
			
			if(FD_ISSET(fd_socket, &rset)) {
				int nRead = 0;	//读取字节
				
				if(0 == (nRead = Read(fd_socket, buf, sizeof(buf)))) {
					Close(fd_socket);
					FD_CLR(fd_socket, &allset);
					clients[i] = -1;
				} else if(nRead > 0) {
					Write(fd_socket, buf, nRead);
					Write(STDOUT_FILENO, buf, nRead);
				}
				
				if(0 == (--nReady)) {
					break;
				}
			}
		}
	} //select - while

	close(fd_listen);
	return ;
}

















