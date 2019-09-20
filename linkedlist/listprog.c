#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
	struct Node *prev, *next;
	double value;	
} NodeL;

typedef struct List {
	struct Node *head, *tail;
}ListL;

int append(ListL* list, double d) {
	if(list == NULL) return -1;
	NodeL* node = (NodeL*)malloc(sizeof(NodeL));
	if(node == NULL) return -1;
	node->value = d;
	node->prev = list->tail;
	node->next = NULL;
	if(list->head == NULL) list->head = node;
	if(list->tail == NULL) list->tail = node;
	else {
		list->tail->next = node;
		list->tail = node;
		}
	return 0;
}

int print(ListL* list){
	NodeL* current = list->head;
	if (current == NULL) puts("the list is empty");
	else {
		while(current != NULL) {
			printf("%.1f ",current->value);
			current = current->next;
		}
		putchar('\n');
	}
	return 0;
}

int erase(ListL* list) {
	NodeL* current = list->tail;
	if(current == NULL) return 0;
	while(current->prev != NULL) {
		current = current->prev;
		free(current->next);
	}
	free(current);
	list->head = NULL;
	list->tail = NULL;
	return 0;
}

int main() {
	ListL list = {NULL,NULL};
	while(1){
		double d;
		if (scanf("%lf",&d) != 1) break;
		if (append(&list,d) != 0) break;
	}
	print(&list);
	erase(&list);
	print(&list);
	return 0;
}
