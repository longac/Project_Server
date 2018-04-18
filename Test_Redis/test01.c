#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <assert.h>

#include <hiredis/hiredis.h>

int
main(
		int argc,
		char **argv
		)
{
	//获取redis操作句柄
	redisContext* c = redisConnect("127.0.0.1", 6379);

	//句柄获取失败处理
	if(c->err) {
		redisFree(c);
		printf("Connect to redisServer faile\n");
		exit(-1);
	}
	printf("Connect to redisServer Success\n");

	//命令一 添加一个set命令
	//键key1 值stanley
	const char* command1 = "set key1 stanley";	//命令
	//获取返回结果
	redisReply* r = (redisReply*) redisCommand(c, command1);

	//如果获取结果为空
	if(NULL == r) {
		redisFree(c);
		printf("Execut command1 failure\n");
		exit(-1);
	}

	//如果结果显示不是OK，出错处理
	if(!(r->type == REDIS_REPLY_STATUS && 0 == strcasecmp(r->str, "OK"))) {
		printf("Failed to execute command[%s]\n", command1);
		freeReplyObject(r);
		redisFree(c);
		exit(-1);
	}
	freeReplyObject(r);
	printf("Succeed to execute command[%s]\n", command1);

	//命令二 得到键为key1 值的长度
	const char* command2 = "strlen key1";
	r = (redisReply*)redisCommand(c, command2);
	//如果结果不是整数出错处理
	if(r->type != REDIS_REPLY_INTEGER) {
		printf("Failed to execute command[%s]\n", command2);
		freeReplyObject(r);
		redisFree(c);
		exit(-1);
	}
	int length = r->integer;
	freeReplyObject(r);
	printf("The length of 'key1' is %d\n", length);
	printf("Succeed to execute command[%s]\n", command2);

	//命令三 获取键为key1的值
	const char* command3 = "get key1";
	r = (redisReply*)redisCommand(c, command3);
	//如果返回结果不是字符串类型
	if(r->type != REDIS_REPLY_STRING) {
		printf("Failed to execute command[%s]\n", command3);
		freeReplyObject(r);
		redisFree(c);
		exit(-1);
	}
	printf("The value if 'key1' is %s\n", r->str);
	freeReplyObject(r);
	printf("Succeed to execute command[%s]\n", command3);

	//命令四 获取键为key2的值
	const char* command4 = "get key2";
	r = (redisReply*)redisCommand(c, command4);
	//如果得到的不为空的处理
	if(r->type != REDIS_REPLY_NIL) {
		printf("Failed to execute command[%s]\n", command4);
		freeReplyObject(r);
		redisFree(c);
		exit(-1);
	}
	freeReplyObject(r);
	printf("Succed to execute command[%s]\n", command4);

	redisFree(c);

	return 0;
}
