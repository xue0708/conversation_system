/***********************************************************************
File：Queue.cpp
Description：队列函数，包括两个功能：入队操作和出队操作
***********************************************************************/
#include "queue.h"
#include <iostream>

/***********************************************************************
Function：result_push
Description：入队操作
Input：result_link：队列；result_node：插入的节点
***********************************************************************/
void result_push(result_link_type* result_link, result_node_datatype* result_node) 
{
	if (result_link->head == NULL)
	{
		result_link->head = result_node;
		result_link->end = result_link->head;
		result_link->result_num++;
	}
	else
	{
		result_link->end->next = result_node;
		result_link->end = result_node;
		result_link->result_num++;
	}
}

/***********************************************************************
Function：result_pop
Description：出队操作
Input：result_link：队列
***********************************************************************/
struct result_node_datatype* result_pop(result_link_type* result_link) 
{
	struct result_node_datatype* tmp_node;
	if (result_link->head == NULL)
	{
		return NULL;
	}
	else if (result_link->head == result_link->end)
	{
		return NULL;
	}
	else
	{
		tmp_node = result_link->head;
		result_link->head = result_link->head->next;
		result_link->result_num--;
		return tmp_node;
	}
}