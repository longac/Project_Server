#include <stdio.h>
#include <unsitd.h>

#define MAX 4096

void send_headers(char* type);

void send_err(错误号, 错误名称, 错误描述);

int main(int argc, const char* argv[])
{
	char line[MAX];
	char method[MAX], path[N], protocol[N];

	char* file;
	struct stat sbuf;
	FILE* fp;
	int ich;

	if(2 != argc)
		send_err();

	if(-1 == chdir(argv[1]))
		send_err();

	//GET /hello.c HTTP/1.1		--> strtok
	if(NULL == (fgets(line, MAX, stdin)))
		send_err();

	//拆分字符串
	//正则表达式匹配
	if(3 != sscanf(line, "%[^ ] %[^ ] %[^ ]", method, path, protocol))
		send_err();

	while(NULL != fgets(line, MAX, stdin))
		if(strcmp(line, "\r\n"))
			break;

	//判断是不是 GET 方法
	if(0 != (strcmp(method, "GET")))
		send_err();

	//判断是不是 / 开头的路径
	if('/' != path[0])
		send_err();

	file = path + 1;
	
	if(stat(file, &sbuf) < 0)
		send_err();

	fp = fopen(file, "r");
	if(NULL == fp)
		send_err();

	//根据不同类别的请求完成指定的回复
	send_headers();

	while(EOF != (ich = getc(fp)))
		putchar(ich);

	fflush(stdout)

	fclose(fp);

	return 0;
}

void send_headers(char* type)
{
	HTTP/1.1 200 OK
	Content-Type: text/plain; charset=iso-8859-1
	Connection: close
	\r\n
}

void send_err(错误号, 错误名称, 错误描述)
{
	http protocol

	<html>
	<head><title>错误号 错误名称</title></head>
	<body>
	错误描述
	</body>
	</html>
}
