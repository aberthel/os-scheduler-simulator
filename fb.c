#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include "proc_queue.h"

/* FOREGROUND-BACKGROUND */


/* returns relevant data for processing */
struct results {
	int pc;
	int t;
	int timer;
	struct ll_node* current;
};


struct linked_list {
	struct ll_node* front;
	struct ll_node* back;
};

struct ll_node{
	struct process *data;
	struct ll_node *next;
	struct ll_node *prev;
};

void add_to_front(struct linked_list *ll, struct process *proc);
void move_older(struct linked_list *ll, struct ll_node *n);
struct ll_node* make_new_ll_node(struct process *p);
void print_ll(struct linked_list *ll);
void remove_from_ll(struct linked_list *ll, struct ll_node* n);

struct results fb_process(struct process *p[], int* results_array[], struct ll_node *c, struct linked_list *ll, struct queue *blocked, int pc, int time_start, int time_estimate, int time, int num_processes);

void fb(struct process *p[], int num_processes) {
	int time_estimate = estimate_time(p, num_processes);
	
	//make array to store process running data
	int* results_array[num_processes];
	
	for(int i=0; i<num_processes; i++) {
		results_array[i] = calloc(time_estimate, sizeof(int));
	}
	
	//this makes it easier to determine what to add onto the queue next
	selection_sort(p, num_processes);
	
	struct linked_list *ll = calloc(1, sizeof(struct linked_list));
	struct queue *blocked =calloc(1, sizeof(struct queue));
	
	int pc = 0; //keeps track of the processes that have entered
	int start_time = 0;
	int end_time = time_estimate;
	int timer = time_slice; //counts down time slice
	

	struct ll_node* current = NULL;
	
	
	struct results r = fb_process(p, results_array, current, ll, blocked, pc, start_time, end_time, timer, num_processes);
	
	
	
	//handles "time over"
	while(processes_completed(p, num_processes) == 0) {
		printf("NOT FINISHED\n");
		start_time += time_estimate;
		end_time += time_estimate;
			
		//increase size of results array
		for(int i=0; i<num_processes; i++) {
			results_array[i] = realloc(results_array[i], sizeof(int)*end_time);
		}
		r = fb_process(p, results_array, r.current, ll, blocked, r.pc, start_time, end_time, r.timer, num_processes);
	} 
		
	print_array_to_file(results_array, r.t, "fb.csv", num_processes);

	printf("Made it out alive!\n");

	//calculate_metrics(results_array, p, r.t, num_processes);
	
	calculate_metrics_groups(results_array, p, r.t, num_processes, 80);

	//TODO: remember to free allocated memory!
	for(int i=0; i<num_processes; i++) {
		free(results_array[i]);
		results_array[i] = NULL;
	}  

}


struct results fb_process(struct process *p[], int* results_array[], struct ll_node *c, struct linked_list *ll, struct queue *blocked, int pc, int time_start, int time_estimate, int time, int num_processes) {
	struct ll_node *current = c;
	int t;
	int timer = time;
	
	for(t=time_start; t<time_estimate; t++) {
		
		if(processes_completed(p, num_processes) == 1) {
			break;
		}
		
		//place newly entered processes in the ready list
		//AND at the back of the queue
		while(pc < num_processes && p[pc]->enter_time <= t) {
			if(p[pc]->status == 0) {
				//printf("Process %d entered\n", p[pc]->id);
				p[pc]->status = 1;
				add_to_front(ll, p[pc]);
				//print_ll(ll);
				//printf("\n");
				
			}
			pc ++;
		}
		
		
		//if there's no process running currently, then get one from the list
		if(current == NULL) {
			current = ll->front;
			timer=time_slice;
			
			if(current != NULL) {
				current->data->status = 3;
			}
		
		}
		
		//set process status to blocked if time for io
		if(current != NULL && is_io_time(current->data)) {
			current->data->status =2;
			current->data->io_timer=0;
			enqueue(current->data, blocked);
		
			remove_from_ll(ll, current);
			current = ll->front;
			timer = time_slice;
			if(current != NULL && current->data->status == 1) {
				current->data->status = 3;
			}
		} 
		
		
		//if CPU time is over, then set status to finished and increment counter
		if(current != NULL && current->data->time_counter == current->data->CPU_time) {
			current->data->status = 4;
			//printf("Process %d completed\n", current->data->id);
			remove_from_ll(ll, current);
			
			//print_ll(ll);
			//printf("\n");
			
			current = ll->front;
			timer = time_slice;
			
			if(current != NULL) {
				current->data->status=3;
			}

		}
		
		if(timer == 0) {	
			move_older(ll, current);
			current->data-> status = 1;
			current = ll->front;
			if(current != NULL) {
				current->data->status=3;
			}
			timer = time_slice;
								
		}
		
		
		if(blocked->front != NULL) {
			
			while(blocked->front != NULL && blocked->front->data->io_timer == io_time) {
				struct process *a = dequeue(blocked);
				a->status = 1;
				add_to_front(ll, a);
				if(ll->front->data == a && current == NULL) {
					current = ll->front;
					current->data->status = 3;
				}
				
			}
		
			struct node* n = blocked->front;
			
			while(n != NULL) {
				n->data->io_timer++;
				n = n->next;
			}
			
		
		}
		
		
		
		//increment time counters
		if(current != NULL) {
			current->data->time_counter++;
			timer --;
		} 
		
		
		//save statuses at this time to file
		for(int i=0; i<num_processes; i++) {
			results_array[p[i]->id][t] = p[i]->status;
		}
	
		
	}
	struct results r = {pc, t, timer, current};
	
	return r;


}


void print_ll(struct linked_list *ll) {
	struct ll_node *n = ll-> front;
	
	while(n != NULL) {
		//printf("Process %d\n", n->data->id);
		n = n->next;
	}
}


void add_to_front(struct linked_list *ll, struct process *proc) {
	struct ll_node *n = make_new_ll_node(proc);
	 
	if(ll->back == NULL) {
		ll->front = n;
		ll->back = n;
	} else {
		n->next = ll->front;
		ll->front->prev = n;
		ll->front = n; //FOR SOME REASON this keeps messing with the first two values of results array...
	}  
	
	move_older(ll, n);

}

struct ll_node* make_new_ll_node(struct process *p) {
	struct ll_node *n = calloc(1, sizeof(struct ll_node));
	n->data = p;
	return n;
}



void remove_from_ll(struct linked_list *ll, struct ll_node* n) {
	
	if(ll->front != NULL) {
		
		if(ll->front == n) {
			if(ll->back == n) { //n is only node in list
				ll->front = NULL;
				ll->back = NULL;
			} else { //n is first node
				ll->front = n->next;
				ll->front->prev = NULL;
			}
			
		} else if(ll->back == n) { //n is the last node
			ll->back = n->prev;
			ll->back->next = NULL;
		} else { //n is a middle node
			n->prev->next = n->next;
			n->next->prev = n->prev;
		}
		
		free(n);
	} 
	
}


//NOTE: assumes that process at front is being passed back
void move_older(struct linked_list *ll, struct ll_node *n) {

	int age = n->data->time_counter;
	
	//if n is the only node in the list, do nothing
	if(ll->back == n) {
		return;
	}
	
	
	//if front node is already the in the right place, do nothing
	if(age < n->next->data->time_counter) {
		return;
	}
	
	struct ll_node *current = n->next;
	
	//else remove node from its place
	
	if(ll->front == n) {
		//n is first node
		ll->front = n->next;
		ll->front->prev = NULL;
	} else { //n is a middle node
		n->prev->next = n->next;
		n->next->prev = n->prev;
	}
	n->next = NULL;
	n->prev = NULL;
	
	
	while(age >= current->data->time_counter) {
		if(current == ll->back) { //if current is end, break out of loop
			current = NULL; //this means that n is the oldest
			break;
		} else {
			current = current->next;
		}
	}
	
	if(current == NULL) { //add node to back
		ll->back->next = n;
		n->prev = ll->back;
		ll->back = n;
	} else { //otherwise, add node to just before current
		current->prev->next = n;
		n->prev = current->prev;
		n->next = current;
		current->prev = n;
	}
	
}
