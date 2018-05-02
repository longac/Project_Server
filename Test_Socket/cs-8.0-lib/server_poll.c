#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <arpa/inet.h>
#include <errno.h>

#include "server_poll.h"
#include "wrap.h"

#define OPEN_MAX 5000
#define MAXLEN 1024

void server_poll(unsigned short port)
{
	int fd_listen = 0;	//监听文件描述符
	int fd_connect = 0;	//客户端连接文件描述符
	int fd_socket = 0;	//处理文件描述符

	struct sockaddr_in addr_serv;
	struct sockaddr_in addr_clie;

	fd_listen = Socket(AF_INET, SOCK_STREAM, 0);

	//端口复用
	int opt;
	setsockopt(fd_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	memset(&addr_serv, 0, sizeof(addr_serv));
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port = htons(port);
	addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
	Bind(fd_listen, (struct sockaddr*)&addr_serv, sizeof(addr_serv));

	Listen(fd_listen, 128);

	//poll init
	struct pollfd clients[OPEN_MAX];	//poll文件描述符结构体数组
	int maxPos = 0;
	int i = 0;

	//init listen event
	clients[0].fd = fd_listen;
	clients[0].events = POLLIN;			//设置监听事件为有数据可读

	//all set fd to -1
	for(i = 1; i < OPEN_MAX; ++i) {
		clients[i].fd = -1;
	}

	int nReady = 0;						//监听客户端有数据数量
	socklen_t addr_clie_len = 0;		//客户端地址长度
	char str[BUFSIZ];					//客户端地址字符串
	char buf[MAXLEN];					//读取缓冲区

	while(1) {
		//阻塞
		nReady = poll(clients, maxPos + 1, -1);
		//fd_listen 有读时间就绪
		if(clients[0].revents & POLLIN) {
			addr_clie_len = sizeof(addr_clie);
			fd_connect = Accept(fd_listen, (struct sockaddr*)&addr_clie, &addr_clie_len);
			printf("[%s:%d] is connected\n",
					inet_ntop(AF_INET, &addr_clie.sin_addr, str, sizeof(str)),
					ntohs(addr_clie.sin_port));
			for(i = 1; i < OPEN_MAX; ++i) {
				if(clients[i].fd < 0) {
					clients[i].fd = fd_connect;
					break;
				}
			}

			if(OPEN_MAX == i) {
				fputs("too many clients\n", stderr);
				printf("please waiting for...\n");
				continue;
			}

			clients[i].events = POLLIN;
			if(i > maxPos) {
				maxPos = i;
			}

			if(--nReady <= 0) {
				continue;
			}
		} //POLLIN if

		for(i = 1; i <= maxPos; ++i) {
			//跳过退出的文件描述符
			if((fd_socket = clients[i].fd) < 0) {
				continue;
			}
			if(clients[i].revents & POLLIN) {
				int nRead = 0;
				if((nRead = Read(fd_socket, buf, sizeof(buf))) < 0) {
					//处理RST标志
					if(ECONNRESET == errno) {
						printf("clients[%d] aborted connection\n", i);
						Close(fd_socket);
						clients[i].fd = -1;
					} else {
						perror("read error: ");
						exit(-1);
					}
				} else if(0 == nRead) {
					printf("clients[%d] closed connection\n", i);
					Close(fd_socket);
					clients[i].fd = -1;
				} else {	//回显示
					Write(fd_socket, buf, nRead);
					Write(STDOUT_FILENO, buf, nRead);
				}

				if(--nReady <= 0) {
					break;
				}
			} //POLLIN if
		}
	} //poll while

	Close(fd_listen);

	return;
}
