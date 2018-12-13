#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include "proc_queue.h"

/* FIRST IN FIRST OUT */


int fifo_process(struct process *p[], int* results_array[], struct queue *q, int pc, int time_start, int time_estimate, int num_processes);

void fifo(struct process *p[], int num_processes) {
	int time_estimate = estimate_time(p, num_processes);
	
	//make array to store process running data
	int* results_array[num_processes];
	
	for(int i=0; i<num_processes; i++) {
		results_array[i] = calloc(time_estimate, sizeof(int));
	}
	
	//sort processes by enter time
	selection_sort(p, num_processes);

	struct queue *q = calloc(1, sizeof(struct queue));
	
	//keeps track of the process currently being run
	int pc = 0;
	int start_time = 0;
	int end_time = time_estimate;

	
	pc = fifo_process(p, results_array, q, pc, start_time, end_time, num_processes);
	
	
	//handles "time over"
	while(processes_completed(p, num_processes) == 0) {
		printf("NOT FINISHED\n");
		start_time += time_estimate;
		end_time += time_estimate;
			
		//increase size of results array
		for(int i=0; i<num_processes; i++) {
			results_array[i] = realloc(results_array[i], sizeof(int)*end_time);
		}
		pc = fifo_process(p, results_array, q, pc, start_time, end_time, num_processes);
	}
		
	
	print_array_to_file(results_array, pc, "fifo.csv", num_processes);

	printf("Made it out alive!\n");

	calculate_metrics(results_array, p, pc, num_processes);
	

	//TODO: remember to free allocated memory!
	for(int i=0; i<num_processes; i++) {
		free(results_array[i]);
		results_array[i] = NULL;
	}  
}

int fifo_process(struct process *p[], int* results_array[], struct queue *q, int pc, int time_start, int time_estimate, int num_processes) {

	int t;
	
	struct process* current = NULL;
	
	for(t=time_start; t<time_estimate; t++) {
		//printf("Time %d\n", t);
		if(processes_completed(p, num_processes) == 1) {
			break;
		}
		
		//place newly entered processes in the ready list
		//AND at the back of the queue
		while(pc < num_processes && p[pc]->enter_time <= t) {
			//printf("Loop\n");
			if(p[pc]->status == 0) {
				//printf("status to 1\n");
				p[pc]->status = 1;
				enqueue(p[pc], q);
			}
			pc ++;
		}
		
		
		
		//if there's no process running currently, then get one from the queue
		if(current == NULL && q->front != NULL) {
			current = q->front->data;
			
			if(current->status == 1) {
				current->status = 3;
			}
		
		}
		
		
		
		//if CPU time is over, then set status to finished and increment counter
		if(current != NULL && current->time_counter == current->CPU_time) {
			current->status = 4;
			
			dequeue(q);
			current = NULL;
			
			if(q->front != NULL) {
				current = q->front->data;
				current->status=3;
			}

		}
		
		if(current != NULL && is_io_time(current) && current->status ==3) {
			current->status =2;
			current->io_timer=0;
		} 
		
		
		if(current!= NULL && current->status ==2) {
			if (current->io_timer == io_time) {
				current->status = 3;
			} else {
				current->io_timer++;
			}
		}
		
		
		
		
		//increment time counters
		if(current != NULL && current->status ==3) {
			current->time_counter++;
		}
		
		
		//save statuses at this time to file
		for(int i=0; i<num_processes; i++) {
			results_array[p[i]->id][t] = p[i]->status;
		}	
	}

	//return t;
	
	if(processes_completed(p, num_processes) == 1) {
		return t;
	} else {
		return pc;
	} 
	

}

