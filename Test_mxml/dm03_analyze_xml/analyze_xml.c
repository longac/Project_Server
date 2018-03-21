#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mxml.h"

int 
main(
	int argc,
	const char* argv[]
	)
{
	FILE* fp = fopen(argv[1], "r");

	if(NULL == fp) {
		perror("fopen error");
		exit(1);
	}

	//将指定内容加载到内存中
	mxml_node_t *xml = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);

	//获取根节点
	mxml_node_t *title = mxmlFindElement(xml, xml, "title", NULL, NULL, MXML_DESCEND);

	if(NULL == title) {
		printf("title not found");
	}
	else {
		//获取节点属性
		printf("title -- addr -- %s\n", mxmlElementGetAttr(title, "address"));
		//获取节点内容
		printf("title -- text -- %s\n", mxmlGetText(title, NULL));
	}

	//删除文件内存
	mxmlDelete(xml);

	//关闭文件
	fclose(fp);
	return 0;
}
