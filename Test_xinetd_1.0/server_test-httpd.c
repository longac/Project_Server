#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX 4096

//日志宏
#define log(info, str) \
	do{\
		fprintf(tmp, "%s%s", info, str);\
		fflush(tmp);\
	}while(0)

//发送协议头
void send_headers(char*, off_t);

//回显错误
void send_err(int , char*, char*);

int main(int argc, const char* argv[])
{
	//读取行缓冲区
	char line[MAX];
	//方法， 请求路径， 协议
	char method[MAX], path[MAX], protocol[MAX];
	//文件名称
	char* file;
	//文件描述
	struct stat sbuf;
	//文件指针
	FILE* fp;
	//
	int ich;
	//文件类型
	char* type;

	//参数传递不全
	if(2 != argc)
		send_err(500, "Internal Error", "Config error - no dir sepcified.");

	//请求目录不正确
	if(-1 == chdir(argv[1]))
		send_err(500, "Internal Error", "Config error - couldn't chdir().");

	//GET /hello.c HTTP/1.1		--> strtok
	if(NULL == (fgets(line, MAX, stdin)))
		send_err(400, "Bad Request", "No request found.");

	//拆分字符串
	//正则表达式匹配
	if(3 != sscanf(line, "%[^ ] %[^ ] %[^ ]", method, path, protocol))
		send_err(400, "Bad Request", "Can't parse request.");

	while(NULL != fgets(line, MAX, stdin))
		if((0 == strcmp(line, "\r\n")) || (0 == strcmp(line, "\n")))
			break;

	//判断是不是 GET 方法
	if(0 != (strcmp(method, "GET")))
		send_err(501, "Not Implemented", "That method is not implemented.");

	//判断是不是 / 开头的路径
	if('/' != path[0])
		send_err(400, "Bad Request", "Bad filename.");

	file = path + 1;	// ‘/'之后是文件名
	
#if 1
	FILE *tmp = fopen("/home/stanley/test-dir/log.i", "a");
	if(NULL == tmp) {
		perror("fopen log.i error");
		exit(1);
	}
#endif
	
	if(stat(file, &sbuf) < 0)
		send_err(404, "Not Found", "File not found.");

	fp = fopen(file, "r");
	if(NULL == fp)
		send_err(403, "Forbidden", "File is protected.");

	char* dot = strrchr(file, '.');
	
	if(NULL == dot) {
		type = "text/plain; charset=utf-8";
	} else if(0 == (strcmp(dot, ".html"))) {
		type = "text/html; charset=utf-8";
	} else if(0 == (strcmp(dot, ".jpg"))) {
		type = "image/jpeg";
	} else if(0 == (strcmp(dot, ".gif"))) {
		type = "image/gif";
	} else if(0 == (strcmp(dot, ".png"))) {
		type = "image/png";
	} else if(0 == (strcmp(dot, ".mp3"))) {
		type = "audio/mpeg";
	} else if(0 == (strcmp(dot, ".mp4"))) {
		type = "video/mpeg4";
	} else {
		type = "text/plain; charset=iso-8859-1";
	}

	//根据不同类别的请求完成指定的回复
	send_headers(type, sbuf.st_size);

	while(EOF != (ich = getc(fp)))
		putchar(ich);

	fflush(stdout);

	fclose(fp);

	return 0;
}

void send_headers(char* type, off_t length)
{
	printf("%s %d %s\r\n", "HTTP/1.1", 200, "Ok");
	printf("Content-Type:%s\r\n", type);
	printf("COntent-Length:%lld\r\n", (int64_t)length);
	printf("Connection: close\r\n");
	printf("\r\n");
}

void send_err(int status, char* title, char* text)
{
	send_headers("test/html", -1);
	printf("<html><head><title>%d %s</title></head>\n", status, title);
	printf("<body bgcolor=\"#cc99cc\"><h4>%d %s</h4>\n", status, title);
	printf("%s\n", text);
	printf("<hr>\n</body>\n</html>\n");
	fflush(stdout);
	exit(1);
	/*
	http protocol
	<html>
	<head><title>错误号 错误名称</title></head>
	<body>
	错误描述
	</body>
	</html>
	*/
}
