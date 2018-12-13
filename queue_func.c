/*
* OS Scheduling Policy Simulator - Process Queue Functions
* Author: Ana Berthel
* Date: 12/13/18
*/
#include <stdlib.h>
#include <stdio.h>
#include "helper.h"


/* makes a new queue node */
struct node* make_new_node(struct process *p) {
	struct node *n = calloc(1, sizeof(struct node));
	n->data = p;
	return n;
}

/* adds a process to the back of the queue */
void enqueue(struct process *p, struct queue *q) {
	struct node *n = make_new_node(p);
	
	if(q->front == NULL) {
		q->back = n;
		q->front = n;
	} else {
		q->back->next = n;
		q->back = n;
	}
}

/* moves the node from the front of the queue to the back*/
void move_to_back(struct queue *q) {
	if(q->front != q->back) {
		struct node *n = q->front;
		q->front = n->next;
		n->next = NULL;
		q->back->next = n;
		q->back = n;
	}
}

/* removes a node from the front of the list and returns its process 
* NOTE: frees memory for the node, but not the process
*/
struct process* dequeue(struct queue *q) {
	if(q->front != NULL) {
		struct node *n = q->front;
		if(q->front == q->back) {
			q->front = NULL;
			q->back = NULL;
		} else {
			q->front = n->next;
		}
		struct process *proc = n->data;
		free(n);
		return proc;
		
	} else {
		return NULL;
	}
}

/* prints a representation of the queue's contents to the console */
void print_queue(struct queue *q) {
	struct node *n = q-> front;
	
	while(n != NULL) {
		printf("Process %d\n", n->data->id);
		n = n->next;
	}
}