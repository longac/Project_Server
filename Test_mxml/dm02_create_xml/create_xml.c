#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mxml.h"

/*
 * 生成xml文件
 */
int main()
{
	//创建并打开一个.xml的文件
	FILE *fp = fopen("test.xml", "w");

	//创建xml的文件
	//创建根节点
	mxml_node_t *tree = mxmlNewXML("1.0");

	//创建一个新元素
	mxml_node_t *node = mxmlNewElement(tree, "city");

	//创建一个新元素
	mxml_node_t *title = mxmlNewElement(node, "title");

	//设置节点属性名和值
	mxmlElementSetAttr(title, "address", "北京");
	
	//创建节点的文本内容
	mxml_node_t *text = mxmlNewText(title, 0, "这是一个test xml");

	//保存xml文件
	mxmlSaveFile(tree, fp, MXML_NO_CALLBACK);
	
	//关闭文件
	fclose(fp);
	return 0;
}
