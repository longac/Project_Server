/*
 * 高并发服务器 epoll-et-nonblock 边缘触发+非阻塞
 * stanley
 * 2018-04-13
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <strings.h>
#include <ctype.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "wrap.h"

#define SERV_PORT 8888
#define OPEN_MAX 5000
#define MAX_LINE 8192
#define ADDR_MSG 32

int
main(
	)
{
	int fd_listen;	//监听文件描述符
	int fd_connect;	//连接文件描述符,用于与客户端通信
	int fd_socket;	//有数据传输的客户端文件描述符
	int fd_epoll;	//epoll文件描述符

	struct sockaddr_in addr_serv;	//服务器地址
	struct sockaddr_in addr_clie;	//客户端地址

	struct epoll_event event;	//临时epoll结点
	struct epoll_event events[OPEN_MAX];	//监听epoll结点集合
	char addr_clie_msg[OPEN_MAX][ADDR_MSG];	//存放客户端地址信息
	
	int ret_epctl = 0;	//上树结果

	int nReady = 0;	//监听反馈数量

	int i = 0;	//遍历监听序列
	int j = 0;	//遍历读取信息转换大小写

	socklen_t addr_clie_len = 0;	//客户端

	char str[INET_ADDRSTRLEN];	//客户端地址字符串 默认16位

	int nRead = 0;	//从客户端读取的数据长度
	char buf[MAX_LINE];	//读取数据缓冲区

	int flag = 0;	//文件阻塞属性

	//设置监听端口
	fd_listen = Socket(AF_INET, SOCK_STREAM, 0);

	//设置端口复用
	int opt;
	setsockopt(fd_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	//服务器地址初始化
	bzero(&addr_serv, sizeof(addr_serv));
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port = htons(SERV_PORT);
	addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
	//绑定
	Bind(fd_listen, (struct sockaddr*)&addr_serv, sizeof(addr_serv));
	//设置监听上限
	Listen(fd_listen, 128);

	//创建epoll 红黑树
	fd_epoll = epoll_create(OPEN_MAX);
	if(-1 == fd_epoll) {
		perror("epoll_create error:");
		exit(-1);
	}
	//设置epoll模式为et
	event.events = EPOLLIN | EPOLLET;
	//event.events = EPOLLIN;
	event.data.fd = fd_listen;

	//监听事件上树
	ret_epctl = epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd_listen, &event);
	if(-1 == ret_epctl) {
		perror("epoll_crl error:");
		exit(-1);
	}

	while(1) {
		nReady = epoll_wait(fd_epoll, events, OPEN_MAX, -1);
		if(-1 == nReady) {
			perror("epoll_wait error:");
			exit(-1);
		}

		//遍历本地监听数组
		for(i = 0; i < nReady; ++i) {
			//判断是否为可读操作
			if(!(EPOLLIN & events[i].events)) {
				continue;
			}
			//连接请求
			if(events[i].data.fd == fd_listen) {
				addr_clie_len = sizeof(addr_clie);

				//等待客户端请求
				fd_connect = Accept(fd_listen, (struct sockaddr*)&addr_clie, &addr_clie_len);

				//客户端连接请求
				sprintf(&addr_clie_msg[i], "%s:%d",
						inet_ntop(AF_INET, &addr_clie.sin_addr, str, sizeof(str)),
						ntohs(addr_clie.sin_port));
				printf("%s is connected...\n", addr_clie_msg[i]);

				//设置fd_connect为非阻塞读
				flag = fcntl(fd_connect, F_GETFL);
				flag |= O_NONBLOCK;
				fcntl(fd_connect, F_SETFL, flag);

				//设置监听事件
				event.events = EPOLLIN | EPOLLET;
				//event.events = EPOLLIN;
				event.data.fd = fd_connect;
				
				//监听事件上树
				ret_epctl = epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd_connect, &event);
				if(-1 == ret_epctl) {
					perror("epoll_ctl error:");
					exit(-1);
				}
			}
			//数据传输
			else {
				fd_socket = events[i].data.fd;
				nRead = Read(fd_socket, buf, MAX_LINE);

				//信息读取
				if(0 == nRead) {	//EOF
					//摘除监听结点
					ret_epctl = epoll_ctl(fd_epoll, EPOLL_CTL_DEL, fd_socket, NULL);
					if(-1 == ret_epctl) {
						perror("epoll_ctl error:");
						exit(-1);
					}
					Close(fd_socket);
					printf("%s is closed...\n", addr_clie_msg[i]);
					strcpy(&addr_clie_msg[i], "");
				}
				else if(nRead < 0) {	//error
					perror("read error:");
					ret_epctl = epoll_ctl(fd_epoll, EPOLL_CTL_DEL, fd_socket, NULL);
					if(-1 == ret_epctl) {
						perror("epoll_ctl error:");
					}
					Close(fd_socket);
					strcpy(&addr_clie_msg[i], "");
				}
				else {	//读到字节数
					//转换读到的信息
					for(j = 0; j < nRead; ++j) {
						buf[j] = toupper(buf[j]);
					}
					Write(STDOUT_FILENO, &addr_clie_msg[i], strlen(addr_clie_msg[i]));
					Write(STDOUT_FILENO, " ", 1);
					Write(STDOUT_FILENO, buf, nRead);
					Write(fd_socket, buf, nRead);
				}
			}
		}
	}

	//关闭监听文件描述符
	close(fd_epoll);
	close(fd_listen);
	return 0;
}
