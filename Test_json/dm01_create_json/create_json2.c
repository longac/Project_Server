#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"

int main(
		int argc,
		const char* argv[]
		)
{
	//创建一个json对象
	cJSON *json = cJSON_CreateObject();

	cJSON_AddStringToObject(json, "author", "stanley");

	//创建一个对象
	cJSON *class = NULL;

	//添加对象到json对象中
	cJSON_AddItemToObject(json, "class", class = cJSON_CreateObject());
	cJSON_AddStringToObject(class, "姓名", "张三");
	cJSON_AddStringToObject(class, "性别", "男");
	cJSON_AddNumberToObject(class, "年龄", 24);

	//将json缓冲到内存中
	char *buf = cJSON_Print(json);

	//创建文件
	FILE *fp = fopen("test2.json", "w");

	//写入文件
	fwrite(buf, 1, strlen(buf), fp);

	//析构内存
	free(buf);
	//关闭文件
	fclose(fp);

	//析构json对象
	cJSON_Delete(json);
	return 0;
}
