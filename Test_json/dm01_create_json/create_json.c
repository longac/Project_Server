#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"

int
main(
		int argc, 
		const char* argv[]
		)
{
	//创建一个cJSON对象
	cJSON *json = cJSON_CreateObject();
	
	//添加两个对象
	cJSON *stu1 = cJSON_CreateArray();
	cJSON *stu2 = cJSON_CreateArray();

	//对象添加到json中
	cJSON_AddItemToObject(json, "学生1", stu1);
	cJSON_AddItemToObject(json, "学生2", stu2);

	//添加属性
	cJSON_AddItemToArray(stu1, cJSON_CreateString("张三"));
	cJSON_AddItemToArray(stu1, cJSON_CreateString("男"));
	cJSON_AddItemToArray(stu1, cJSON_CreateNumber(21));

	cJSON_AddItemToArray(stu2, cJSON_CreateString("李四"));
	cJSON_AddItemToArray(stu2, cJSON_CreateString("男"));
	cJSON_AddItemToArray(stu2, cJSON_CreateNumber(25));


	//将json结构写入缓冲区
	char *buf = cJSON_Print(json);

	//打开文件写入
	FILE *fp = fopen("test.json", "w");
	fwrite(buf, 1, strlen(buf), fp);
	free(buf);
	fclose(fp);

	//析构json对象
	cJSON_Delete(json);

	return 0;
}	
