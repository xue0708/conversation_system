/***********************************************************************
File：Queue.h
Description：队列函数
***********************************************************************/
#ifndef QUEUE_H
#define QUEUE_H
#include <stdio.h>
#include <stdlib.h>
#include "header.h"
using namespace std;

/***********************************************************************
Function：result_node_datatype
Description：队列中的节点成员
***********************************************************************/
struct result_node_datatype
{
	AVBufferRef *buf;
	AVPacket packet;
	unsigned char *result;
	struct result_node_datatype* next;
	int size;
};

/***********************************************************************
Function：result_node_datatype
Description：头结点、尾节点、队列中节点的数目
***********************************************************************/
typedef struct result_link_datatype
{
	struct result_node_datatype *head;
	struct result_node_datatype *end;
	int result_num;

}result_link_type;

/***********************************************************************
Function：result_node_datatype
Description：实现的两个队列函数
result_push：入队操作
result_pop：出队操作
***********************************************************************/
void result_push(result_link_type* result_link, result_node_datatype* result_node); 
struct result_node_datatype* result_pop(result_link_type* result_link);

#endif