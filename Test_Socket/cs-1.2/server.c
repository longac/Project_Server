#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <ctype.h>
#include <arpa/inet.h>

#include "wrap.h"

/*
 * 服务器端代码
 * stanley
 * 2018-03-25
 * 转换客户端发来的消息回给客户端
 */

#define SERV_PORT 6666

int
main(
		int argc,
		const char* argv[]
	)
{
	int lfd = 0;
	int cfd = 0;
	struct sockaddr_in serv_addr, clie_addr;
	socklen_t clie_addr_len = 0;
	int i = 0;
	char buf[BUFSIZ];
	char clie_buff[BUFSIZ];
	int n;

	//创建套接字
	lfd = Socket(AF_INET, SOCK_STREAM, 0);

	serv_addr.sin_family = AF_INET;					//协议
	serv_addr.sin_port = htons(SERV_PORT);			//端口号
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);	//IP地址

	//绑定
	Bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	//设置监听上线
	Listen(lfd ,128);

	//传入传出参数
	clie_addr_len = sizeof(clie_addr);
	cfd = Accept(lfd, (struct sockaddr*)&clie_addr, &clie_addr_len);

	printf("client IP : %s  client PORT : %d\n",
			inet_ntop(AF_INET, &clie_addr.sin_addr, clie_buff, sizeof(clie_buff)),
			ntohs(clie_addr.sin_port));

	while(i < 5) {
		n = Read(cfd, buf, sizeof(buf));
		for(int j = 0; j < n; ++j) {
			buf[j] = toupper(buf[j]);
		}
		Write(cfd, buf, n);
		i++;
	}

	Close(lfd);
	Close(cfd);

	return 0;
}
