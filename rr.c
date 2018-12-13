/*
* OS Scheduling Policy Simulator - Round Robin Implementation
* Author: Ana Berthel
* Date: 12/13/18
*/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include "helper.h"

//returns relevant data about round of simulation
struct results {
	int pc;
	int t;
	int timer;
	struct process* current;
};

//methods
struct results rr_process(struct process *p[], int* results_array[], struct process *c, struct queue *q, struct queue *blocked, int pc, int time_start, int time_estimate, int time, int num_processes);

/* Main RR method. Sets up the RR simulation. */
void rr(struct process *p[], int num_processes) {
	
	//get time estimate
	int time_estimate = estimate_time(p, num_processes);
	
	//make array to store process running data
	int* results_array[num_processes];
	for(int i=0; i<num_processes; i++) {
		results_array[i] = calloc(time_estimate, sizeof(int));
	}
	
	//sort processes by enter time
	selection_sort(p, num_processes);
	
	//initialize ready process queue and blocked process queue
	struct queue *q = calloc(1, sizeof(struct queue));
	struct queue *blocked =calloc(1, sizeof(struct queue));
	
	//keeps track of the process currently being run
	int pc = 0; 
	int start_time = 0;
	int end_time = time_estimate;
	int timer = time_slice; 
	struct process* current = NULL;

	//run first round of RR simulation
	struct results r = rr_process(p, results_array, current, q, blocked, pc, start_time, end_time, timer, num_processes);
	
	//if first round does not result in all processes being completed, run additional rounds
	while(processes_completed(p, num_processes) == 0) {
		//increase time estimate
		start_time += time_estimate;
		end_time += time_estimate;
			
		//increase size of results array
		for(int i=0; i<num_processes; i++) {
			results_array[i] = realloc(results_array[i], sizeof(int)*end_time);
		}
		
		//run next round of RR simulation
		r = rr_process(p, results_array, r.current, q, blocked, r.pc, start_time, end_time, r.timer, num_processes);
	}
	
	//write results array to file	
	print_array_to_file(results_array, r.t, "rr.csv", num_processes);

	//calculate metrics
	printf("Simulation complete!\n\n");
	calculate_metrics(results_array, p, r.t, num_processes);

	//free allocated memory
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

/* Auxilliary RR Method. Contains main RR logic */
struct results rr_process(struct process *p[], int* results_array[], struct process *c, struct queue *q, struct queue *blocked, int pc, int time_start, int time_estimate, int time, int num_processes) {
	
	int t;
	int timer = time;
	struct process *current = c;
	
	//for each time in time estimate, determine status of each process
	for(t=time_start; t<time_estimate; t++) {
		
		//if all processes are complete, no need to go further
		if(processes_completed(p, num_processes) == 1) {
			break;
		}
		
		//place newly entered processes in the ready queue and set status to 1
		while(pc < num_processes && p[pc]->enter_time <= t) {
			if(p[pc]->status == 0) {
				p[pc]->status = 1;
				enqueue(p[pc], q);
			}
			pc ++;
		}
		
		//if no process is currently running, get the process at the front of the queue to run
		if(current == NULL) {
			current = dequeue(q);
			timer=time_slice;
			
			if(current != NULL) {
				current->status = 3;
			}
		
		}
			
		//if current process makes IO request, set status to blocked and start IO timer
		if(current != NULL && current->status == 3 && is_io_time(current)) {
			current->status =2;
			current->io_timer=0;
			enqueue(current, blocked);
			
			//get next process in queue to run
			current = dequeue(q);
			timer = time_slice;
			if(current != NULL && current->status == 1) {
				current->status = 3;
			}
		} 
			
		//if current process's CPU time is over, then set status to finished and get next process to run
		if(current != NULL && current->time_counter == current->CPU_time) {
			current->status = 4;
			
			current = dequeue(q);
			if(current != NULL && current->status ==1) {
				current->status=3;
			}
			timer = time_slice;
		}
			
		//if time slice is over, then move current process to back of queue
		//and get new process to run
		if(timer == 0) {		
			enqueue(current, q);
			current-> status = 1;
			current = dequeue(q);
			if(current != NULL && current->status ==1) {
				current->status=3;
			}
			timer = time_slice;					
		}

		//if there are blocked processes
		if(blocked->front != NULL) {
			
			//remove processes that have completed IO request from blocked queue
			//and place them at the end of the ready queue
			while(blocked->front != NULL && blocked->front->data->io_timer == io_time) {
				struct process *a = dequeue(blocked);
				a->status = 1;
				enqueue(a, q);
				if(q->front->data == a && current == NULL) {
					current = dequeue(q);
					current->status = 3;
				}
			}
		
			//increment IO timer on remaining blocked processes
			struct node* n = blocked->front;
			while(n != NULL) {
				n->data->io_timer++;
				n = n->next;
			}
		}
		
		
		//increment time counter for running process
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
