#include "stdafx.h"


void list_new(List *list, int elementSize, freeFunction freeFn)
{

	assert(elementSize > 0);
	list->logicalLength = 0;
	list->elementSize = elementSize;
	list->head = list->tail = NULL;
	list->freeFn = freeFn;
	InitializeCriticalSection(&(list)->cs);

}

void list_destroy(List *list)
{
	ListNode *current;
	while (list->head != NULL) {
		current = list->head;
		list->head = current->next;

		if (list->freeFn) {
			list->freeFn(current->data);
		}

		free(current->data);
		free(current);
	}

	DeleteCriticalSection(&(list)->cs);

}

void list_prepend(List *list, void *element)
{
	EnterCriticalSection(&list->cs);

	ListNode *node = (ListNode*)malloc(sizeof(ListNode));
	node->data = malloc(list->elementSize);
	memcpy(node->data, element, list->elementSize);

	node->next = list->head;
	list->head = node;

	// first node?
	if (!list->tail) {
		list->tail = list->head;
	}

	list->logicalLength++;
	LeaveCriticalSection(&list->cs);

}

ListNode* list_append(List *list, void *element)
{
	EnterCriticalSection(&list->cs);

	ListNode *node = (ListNode*)malloc(sizeof(ListNode));
	node->data = malloc(list->elementSize);
	node->next = NULL;

	memcpy(node->data, element, list->elementSize);

	if (list->logicalLength == 0) {
		list->head = list->tail = node;
	}
	else {
		list->tail->next = node;
		list->tail = node;
	}

	list->logicalLength++;
	LeaveCriticalSection(&list->cs);
	return node;
}

void list_for_each_param(List *list, paramIterator iterator, void* param)
{
	EnterCriticalSection(&list->cs);

	assert(iterator != NULL);

	ListNode *node = list->head;
	bool result = true;
	while (node != NULL && result) {
		result = iterator(node, param);
		node = node->next;
	}
	LeaveCriticalSection(&list->cs);
}

void list_for_each(List *list, listIterator iterator)
{
	EnterCriticalSection(&list->cs);

	assert(iterator != NULL);

	ListNode *node = list->head;
	bool result = true;
	while (node != NULL && result) {
		result = iterator(node->data);
		node = node->next;
	}
	LeaveCriticalSection(&list->cs);
}

void list_head(List *list, void *element, bool removeFromList)
{
	EnterCriticalSection(&list->cs);

	assert(list->head != NULL);

	ListNode *node = list->head;
	memcpy(element, node->data, list->elementSize);

	if (removeFromList) {
		list->head = node->next;
		list->logicalLength--;

		free(node->data);
		free(node);
	}

	LeaveCriticalSection(&list->cs);
}

void list_tail(List *list, void *element)
{
	EnterCriticalSection(&list->cs);

	assert(list->tail != NULL);
	ListNode *node = list->tail;
	memcpy(element, node->data, list->elementSize);

	LeaveCriticalSection(&list->cs);
}

int list_size(List *list)
{
	return list->logicalLength;
}

void* list_find(List *list, void *element, listCompare comparator) {
	EnterCriticalSection(&list->cs);

	ListNode *node = list->head;
	while (node != NULL) {
		if (comparator(node, element)) {
			LeaveCriticalSection(&list->cs);
			return node;
		}
		node = node->next;
	}

	LeaveCriticalSection(&list->cs);
	return NULL;
}