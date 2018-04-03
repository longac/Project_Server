#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>

#include "wrap.h"

#define MAXSIZE 5
#define SERV_PORT 8888
#define SERV_IP "127.0.0.1"

//回调函数 子线程处理函数
void* child_do(void* arg);

int
main(
		int argc,
		const char** argv
		)
{
	pthread_t pthids[MAXSIZE];
	int i = 0;
	int ret = 0;

	for(i = 0; i < MAXSIZE; ++i) {
		ret = pthread_create(&pthids[i], NULL, child_do, NULL);
		if(0 != ret) {
			printf("child thread error %d\n", ret);
			printf("%s\n", strerror(ret));
		}

		pthread_detach(pthids[i]);
	}


	return 0;
}

void*
child_do(
		void* arg
		)
{
	int fd_connect = 0;
	struct sockaddr_in addr_serv;
	char buf[BUFSIZ];
	ssize_t n = 0;

	fd_connect = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&addr_serv, sizeof(addr_serv));
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, SERV_IP, &addr_serv.sin_addr.s_addr);
	Connect(fd_connect, (struct sockaddr*)&addr_serv, sizeof(addr_serv));

	while(1) {
		strcpy(buf, "hello world\n");
		Write(fd_connect, buf, strlen(buf));
		n = Read(fd_connect, buf, sizeof(buf));
		Write(STDOUT_FILENO, buf, n);
		sleep(2);
	}

	Close(fd_connect);
	return (void*)0;
}
