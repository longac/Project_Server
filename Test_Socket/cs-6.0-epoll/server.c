/*
 * epoll模型初版
 * stanley
 * 2018-04-03
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <ctype.h>
#include <sys/epoll.h>

#include "wrap.h"

#define SERV_PORT 8888
#define OPEN_MAX 5000
#define MAXLINE 8192

int
main(
		int argc,
		const char* argv[]
		)
{
	int fd_listen, fd_connect, fd_socket;
	int ret = 0;	//epoll添加结点的返回值
	ssize_t nReady, fd_epoll; 
	struct sockaddr_in addr_serv, addr_clie;
	socklen_t addr_clie_len;
	//temp epoll_ctl参数
	//ep epool_wait参数
	struct epoll_event temp, ep[OPEN_MAX];	//epoll事件临时

	int i = 0;
	int num = 0;	//客户端连接数量
	char buf[MAXLINE], str[INET_ADDRSTRLEN];	//INET_ADDRSTRLEN 16
	int nRead = 0;	//客户端读取的数据长度

	fd_listen = Socket(AF_INET, SOCK_STREAM, 0);

	//端口复用
	int opt = 1;
	setsockopt(fd_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	//数据初始化
	bzero(&addr_serv, sizeof(addr_serv));
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port = htons(SERV_PORT);
	addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//绑定端口
	Bind(fd_listen, (struct sockaddr*)&addr_serv, sizeof(addr_serv));

	//设置监听数量上线
	Listen(fd_listen, 128);

	//创建epoll句柄
	fd_epoll = epoll_create(OPEN_MAX);
	if(-1 == fd_epoll) {
		perror("epoll_create error:");
		exit(-1);
	}

	temp.events = EPOLLIN;
	temp.data.fd = fd_listen;	//指定fd_listen的监听事件为 读

	//将监听事件挂到监听树上
	ret = epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd_listen, &temp);
	if(-1 == ret) {
		perror("epoll_ctl error:");
		exit(-1);
	}

	while(1) {
		nReady = epoll_wait(fd_epoll, ep, OPEN_MAX, -1);
		//对nReady返回值进行判断
		if(-1 == nReady) {
			perror("epoll_wait error:");
			exit(-1);
		}

		//遍历监听数组
		for(i = 0; i < nReady; ++i) {
			//判断是否为可读事件
			if(!(ep[i].events & EPOLLIN)) {
				continue;
			}

			//判断是否为三次握手
			if(ep[i].data.fd == fd_listen) {
				addr_clie_len = sizeof(addr_clie);
				fd_connect = Accept(fd_listen, (struct sockaddr*)&addr_clie, &addr_clie_len);

				printf("received from %s at PORT %d\n",
						inet_ntop(AF_INET, &addr_clie.sin_addr, str, sizeof(str)),
						ntohs(addr_clie.sin_port));
				printf("fd_connect %d ---client %d\n", fd_connect, ++num);

				temp.events = EPOLLIN;
				temp.data.fd = fd_connect;
				ret = epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd_connect, &temp);
				if(-1 == ret) {
					perror("epoll_ctl error:");
					exit(-1);
				}
			}
			//不是做读取操作
			else {
				fd_socket = ep[i].data.fd;
				nRead = Read(fd_socket, buf, MAXLINE);
				//信息读取完毕
				if(0 == nRead) {
					ret = epoll_ctl(fd_epoll, EPOLL_CTL_DEL, fd_socket, NULL);
					if(-1 == ret) {
						perror("epoll_ctl error:");
						exit(-1);
					}
					Close(fd_socket);
					printf("client[%d] closed connection.\n", fd_socket);
					num--;
				}
				else if(nRead < 0) {
					perror("read n < 0 error:");
					ret = epoll_ctl(fd_epoll, EPOLL_CTL_DEL, fd_socket, NULL);
					Close(fd_socket);
					num--;
				}
				else {
					for(i = 0; i < nRead; ++i) {
						buf[i] = toupper(buf[i]);
					}
					Write(STDOUT_FILENO, buf, nRead);
					Writen(fd_socket, buf, nRead);
				}
			}
		}
	}
	Close(fd_listen);
	Close(fd_epoll);

	return 0;
}
