/*
 * 高并发服务器
 * stanley
 * 2018-04-23
 *
 * 目前支持select
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"

int
main(int argc, char** argv)
{
	unsigned short port = PORT_SERV;

	//如果用户传递参数，接收作为端口
	if(argc >= 2) {
		port = atoi(argv[1]);
		printf("config -- port = %d\n", port);
		if(argc == 3) {
			if(0 == strcmp("select", argv[2])) {
				printf("config -- method = %s\n", argv[2]);
				server_select(port);
			} else if(0 == strcmp("poll", argv[2])) {
				printf("config -- method = %s\n", argv[2]);
				server_poll(port);
			} else {}
		}
	}

	return 0;
}






