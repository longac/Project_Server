#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

/*
 * 客户端
 * stanley
 * 2018-03-23
 * 发送信息给服务器，将从服务器收到的信息，打印到屏幕上
 */

#define SERV_PORT 6666
#define SERV_IP "127.0.0.1"

int
main(
		int argc,
		const char* argv[]
	)
{
	int cfd = 0;
	struct sockaddr_in serv_addr;
	char buf[BUFSIZ];
	ssize_t n = 0; 

	cfd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr.s_addr);

	//与服务器建立连接
	connect(cfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	while(1) {
		//hello world  -->  hello world\n\0
		fgets(buf, sizeof(buf), stdin);
		write(cfd, buf, strlen(buf));
		n = read(cfd, buf, sizeof(buf));
		write(STDOUT_FILENO, buf, n);
	}

	close(cfd);

	return 0;
}
