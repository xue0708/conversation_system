/***********************************************************************
File��Queue.h
Description�����к���
***********************************************************************/
#ifndef QUEUE_H
#define QUEUE_H
#include <stdio.h>
#include <stdlib.h>
#include "header.h"
using namespace std;

/***********************************************************************
Function��result_node_datatype
Description�������еĽڵ��Ա
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
Function��result_node_datatype
Description��ͷ��㡢β�ڵ㡢�����нڵ����Ŀ
***********************************************************************/
typedef struct result_link_datatype
{
	struct result_node_datatype *head;
	struct result_node_datatype *end;
	int result_num;

}result_link_type;

/***********************************************************************
Function��result_node_datatype
Description��ʵ�ֵ��������к���
result_push����Ӳ���
result_pop�����Ӳ���
***********************************************************************/
void result_push(result_link_type* result_link, result_node_datatype* result_node); 
struct result_node_datatype* result_pop(result_link_type* result_link);

#endif