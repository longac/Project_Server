#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <ctype.h>

#include "wrap.h"

#define SERV_PORT 8888

int
main(
		int argc,
		const char* argv[]
		)
{
	int fd_listen;
	int fd_connect;
	int fd_socket;	//当前要处理的客户端描述符
	int fd_max;		//当前使用的最大文件描述符
	struct sockaddr_in addr_serv;
	struct sockaddr_in addr_clie;
	socklen_t addr_clie_len;	//客户端地址长度
	fd_set rset;	//读事件文件描述符
	fd_set allset;	//暂存所有连接的文件描述符

	int nReady = 0;	//客户端有读请求的数量
	int clients[FD_SETSIZE];	//本地文件描述符数组
	int pos = 0;	//本地文件描述符数组下标

	char buf[BUFSIZ];	//读取缓冲区
	char str[INET_ADDRSTRLEN];	//地址字符串类型 INET_ADDRSTRLEN 16

	int opt = 1;
	int i = 0;

	fd_listen = Socket(AF_INET, SOCK_STREAM, 0);

	//设置端口复用
	setsockopt(fd_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	bzero(&addr_serv, sizeof(addr_serv));
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port = htons(SERV_PORT);
	addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
	Bind(fd_listen, (struct sockaddr*)&addr_serv, sizeof(addr_serv));

	//设置监听上线
	Listen(fd_listen, 128);

	fd_max = fd_listen;	//设置当前使用的最大文件描述符
	pos = -1;	//下标初始化成-1	

	//本地文件描述符数组初始化
	memset(clients, -1, sizeof(clients));

	FD_ZERO(&allset);	//初始化所有连接文件描述符集合
	FD_SET(fd_listen, &allset);	//构造select监听文件描述符集合
	
	while(1) {
		rset = allset;	//文件描述符备忘录
		//fd_max + 1 当前未用的最小文件描述符
		nReady = select(fd_max + 1, &rset, NULL, NULL, NULL);
		if(nReady < 0) {
			perror("select error");
			exit(-1);
		}
		
		//如果当前监听的文件描述符有读时间
		if(FD_ISSET(fd_listen, &rset)) {
			//由内核移交数据到服务器
			addr_clie_len = sizeof(addr_clie);
			fd_connect = Accept(fd_listen, (struct sockaddr*)&addr_clie, &addr_clie_len);
			printf("received from %s at PORT %d\n",
					inet_ntop(AF_INET, &addr_clie.sin_addr, str, sizeof(str)),
					ntohs(addr_clie.sin_port));

			//寻找本地文件描述符数组中，未使用的位置
			for(i = 0; i < FD_SETSIZE; ++i) {
				if(clients[i] < 0) {
					//找到未使用的描述符
					//将与客户端通信的描述符添加到数组中
					clients[i] = fd_connect;
					break;
				}
			}

			if(FD_SETSIZE == i) {
				fputs("too many clients\n", stderr);
				//exit(1);
			}

			FD_SET(fd_connect, &allset);	//向监控文件描述集合添加新的文件描述符fd_connect

			//如果当前描述符大于了最大的描述符，更新最大的文件描述符
			if(fd_connect > fd_max) {
				fd_max = fd_connect;
			}

			if(i > pos) {
				pos = i;
			}

			//所有的读监听处理完毕
			if(0 == (--nReady)) {
				continue;
			}
		}

		for(i = 0; i <= pos; ++i) {
			if((fd_socket = clients[i]) < 0) {
				continue;
			}
			//如果监听到由监听读事件
			if(FD_ISSET(fd_socket, &rset)) {
				int n = 0;	//读取的字节
				//所有数据已经读完
				if(0 == (n = Read(fd_socket, buf, sizeof(buf)))) {
					Close(fd_socket);
					//解除select对当前描述符的监听
					FD_CLR(fd_socket, &allset);
					clients[i] = -1;
				}
				//如果由数据
				else if(n > 0) {
					//完成业务逻辑
					//小写字符转大写字符
					for(int j = 0; j < n; ++j) {
						buf[j] = toupper(buf[j]);
					}
					Write(fd_socket, buf, n);
					Write(STDOUT_FILENO, buf, n);
				}
				if(0 == (--nReady)) {
					break;
				}
			}
		}

	} //--select while--

	Close(fd_listen);
	return 0;
}
