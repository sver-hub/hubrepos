#include <stdio.h>
#include <stdlib.h>

typedef struct Node 
{
	struct Node *prev, *next;
	double value;	
} NodeL;

typedef struct List 
{
	struct Node *head, *tail;
} ListL;

int append(ListL* list, double d) 
{
	if(list == NULL) return -1;
	NodeL* node = (NodeL*)malloc(sizeof(NodeL));
	if(node == NULL) return -1;
	node->value = d;
	node->prev = list->tail;
	node->next = NULL;
	if(list->head == NULL) list->head = node;
	if(list->tail == NULL) list->tail = node;
	else 
	{
		list->tail->next = node;
		list->tail = node;
	}
	return 0;
}

int print(ListL* list)
{
	NodeL* current = list->head;
	if (current == NULL) puts("the list is empty");
	else 
	{
		while(current != NULL) 
		{
			printf("%.1f ",current->value);
			current = current->next;
		}
		putchar('\n');
	}
	return 0;
}

int removeNeg(ListL* list)
{
	NodeL* current = list->head->next;
	NodeL* tmp;
	while(current->next != NULL) 
	{
		if(current->value < 0) 
		{
			tmp = current;
			current->prev->next = current->next;
			current->next->prev = current->prev;
			current = current->next;
			free(tmp);
		} 
		else current = current->next;
	}
	if(list->head->value < 0)
	{
		current = list->head->next;
		free(current->prev);
		list->head = current;
		current->prev = NULL;
	}
	if(list->tail->value < 0) 
	{
		current = list->tail->prev;
		free(current->next);
		current->next = NULL;
		list->tail = current;
	}
	return 0;
}

int erase(ListL* list) 
{
	NodeL* current = list->tail;
	if(current == NULL) return 0;
	while(current->prev != NULL) 
	{
		current = current->prev;
		free(current->next);
	}
	free(current);
	list->head = NULL;
	list->tail = NULL;
	return 0;
}

int main() 
{
	ListL list = {NULL,NULL};
	while(1)
	{
		double d;
		if (scanf("%lf",&d) != 1) break;
		if (append(&list,d) != 0) break;
	}
	print(&list);
	removeNeg(&list);
	print(&list);
	erase(&list);
	print(&list);
	return 0;
}
