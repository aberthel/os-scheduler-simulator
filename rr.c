#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include "proc_queue.h"

/* ROUND ROBIN */


/* returns relevant data for processing */
struct results {
	int pc;
	int t;
	int timer;
	struct process* current;
};

struct results rr_process(struct process *p[], int* results_array[], struct process *c, struct queue *q, struct queue *blocked, int pc, int time_start, int time_estimate, int time, int num_processes);


void rr(struct process *p[], int num_processes) {
	int time_estimate = estimate_time(p, num_processes);
	
	//make array to store process running data
	int* results_array[num_processes];
	
	for(int i=0; i<num_processes; i++) {
		results_array[i] = calloc(time_estimate, sizeof(int));
	}
	
	//this makes it easier to determine what to add onto the queue next
	selection_sort(p, num_processes);
	
	struct queue *q = calloc(1, sizeof(struct queue));
	struct queue *blocked =calloc(1, sizeof(struct queue));
	
	int pc = 0; //keeps track of the processes that have entered
	int start_time = 0;
	int end_time = time_estimate;
	int timer = time_slice; //counts down time slice
	
	//int t;
	struct process* current = NULL;

	struct results r = rr_process(p, results_array, current, q, blocked, pc, start_time, end_time, timer, num_processes);
	
	
	//handles "time over"
	while(processes_completed(p, num_processes) == 0) {
		printf("NOT FINISHED\n");
		start_time += time_estimate;
		end_time += time_estimate;
			
		//increase size of results array
		for(int i=0; i<num_processes; i++) {
			results_array[i] = realloc(results_array[i], sizeof(int)*end_time);
		}
		r = rr_process(p, results_array, r.current, q, blocked, r.pc, start_time, end_time, r.timer, num_processes);
	}
		
	print_array_to_file(results_array, r.t, "rr.csv", num_processes);

	printf("Made it out alive!\n");

	//calculate_metrics(results_array, p, r.t, num_processes);
	
	calculate_metrics_groups(results_array, p, r.t, num_processes, 80);

	//TODO: remember to free allocated memory!
	for(int i=0; i<num_processes; i++) {
		free(results_array[i]);
		results_array[i] = NULL;
	}  

	while(q->front != NULL) {
		dequeue(q);
	}	

	free(q);
	q = NULL; 

	free(blocked);
	blocked = NULL;
}

struct results rr_process(struct process *p[], int* results_array[], struct process *c, struct queue *q, struct queue *blocked, int pc, int time_start, int time_estimate, int time, int num_processes) {
	int t;
	int timer = time;
	struct process *current = c;
	
	for(t=time_start; t<time_estimate; t++) {
		
		if(processes_completed(p, num_processes) == 1) {
			break;
		}
		
		//place newly entered processes in the ready list
		//AND at the back of the queue
		while(pc < num_processes && p[pc]->enter_time <= t) {
		
			if(p[pc]->status == 0) {
				p[pc]->status = 1;
				enqueue(p[pc], q);
			}
			pc ++;
		}
		
		//if there's no process running currently, then get one from the queue
		if(current == NULL) {
			current = dequeue(q);
			timer=time_slice;
			
			if(current != NULL) {
				current->status = 3;
			}
		
		}
		
		
		if(current != NULL && current->status == 3) { //if front of queue is able to run
			
			//set process status to blocked if time for io
			if(is_io_time(current)) {
				//printf("Process %d to io\n", current->id);
				//printf("Process time: %d\n", current->time_counter);
				current->status =2;
				current->io_timer=0;
				enqueue(current, blocked);
			
				//enqueue(current, q); //send to back of list
			
				current = dequeue(q);
				timer = time_slice;
				if(current != NULL && current->status == 1) {
					current->status = 3;
				}
			} 
			
			//if CPU time is over, then set status to finished and increment counter
			if(current != NULL && current->time_counter == current->CPU_time) {
				current->status = 4;
			
				current = dequeue(q);
				if(current != NULL && current->status ==1) {
					current->status=3;
				}
				timer = time_slice;

			}
			
			
			//if time slice is over, then put current running status to back
			if(timer == 0) {
				
				enqueue(current, q);
				current-> status = 1;
				current = dequeue(q);
				if(current != NULL && current->status ==1) {
					current->status=3;
				}
				timer = time_slice;
								
			}
			
		
		}
		
		
		if(blocked->front != NULL) {
			
			while(blocked->front != NULL && blocked->front->data->io_timer == io_time) {
				//blocked->front->data->status = 1;
				struct process *a = dequeue(blocked);
				a->status = 1;
				enqueue(a, q);
				if(q->front->data == a && current == NULL) {
					current = dequeue(q);
					current->status = 3;
					//printf("Process %d running\n", a->id);
				}
				
			}
		
			struct node* n = blocked->front;
			
			while(n != NULL) {
				n->data->io_timer++;
				n = n->next;
			}
			
		
		}
		
		
		//increment time counters
		if(current != NULL && current->status ==3) {
			current->time_counter++;
			//printf("Process %d: time %d\n", current->id, current->time_counter);
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



