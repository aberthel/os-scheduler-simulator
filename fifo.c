/*
* OS Scheduling Policy Simulator - First In First Out Implementation
* Author: Ana Berthel
* Date: 12/13/18
*/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include "helper.h"

//methods
int fifo_process(struct process *p[], int* results_array[], struct queue *q, int pc, int time_start, int time_estimate, int num_processes);

/* Main FIFO method. Sets up the FIFO simulation. */
void fifo(struct process *p[], int num_processes) {
	
	//get time estimate
	int time_estimate = estimate_time(p, num_processes);
	
	//make array to store process running data
	int* results_array[num_processes];
	for(int i=0; i<num_processes; i++) {
		results_array[i] = calloc(time_estimate, sizeof(int));
	} 
	
	//sort processes by enter time
	selection_sort(p, num_processes);
	
	//initialize process queue
	struct queue *q = calloc(1, sizeof(struct queue));
	
	//keeps track of the process currently being run
	int pc = 0;
	int start_time = 0;
	int end_time = time_estimate;
	
	//run first round of FIFO simulation
	pc = fifo_process(p, results_array, q, pc, start_time, end_time, num_processes);
	
	//if first round does not result in all processes being completed, run additional rounds
	while(!processes_completed(p, num_processes)) {
		//increase time estimate
		start_time += time_estimate;
		end_time += time_estimate;
			
		//increase size of results array
		for(int i=0; i<num_processes; i++) {
			results_array[i] = realloc(results_array[i], sizeof(int)*end_time);
		}
		
		//run next round of FIFO simulation
		pc = fifo_process(p, results_array, q, pc, start_time, end_time, num_processes);
	}
		
	//write results array to file
	print_array_to_file(results_array, pc, "fifo.csv", num_processes);

	//calculate metrics
	printf("Simulation complete!\n\n");
	calculate_metrics(results_array, p, pc, num_processes);
	
	
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
}

/* Auxilliary FIFO Method. Contains main FIFO logic */
int fifo_process(struct process *p[], int* results_array[], struct queue *q, int pc, int time_start, int time_estimate, int num_processes) {

	int t;
	struct process* current = NULL;
	
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
		
		//if no process is currently running or blocked, get the process at the front of the queue to run
		if(current == NULL && q->front != NULL) {
			current = q->front->data;
			if(current->status == 1) {
				current->status = 3;
			}
		}
		
		//if current process's CPU time is over, then set status to finished get next process to run
		if(current != NULL && current->time_counter == current->CPU_time) {
			current->status = 4;		
			dequeue(q);
			current = NULL;
	
			if(q->front != NULL) {
				current = q->front->data;
				current->status=3;
			}
		}
		
		//if current process makes IO request, set status to blocked and start IO timer
		if(current != NULL && is_io_time(current) && current->status ==3) {
			current->status =2;
			current->io_timer=0;
		} 
		
		//if current process is blocked
		if(current!= NULL && current->status ==2) {
			if (current->io_timer == io_time) { //unblock if IO request is completed
				current->status = 3;
			} else { //increment IO timer
				current->io_timer++;
			}
		}
		
		//increment time counter for running process
		if(current != NULL && current->status ==3) {
			current->time_counter++;
		}
		
		
		//save statuses at this time to file
		for(int i=0; i<num_processes; i++) {
			results_array[p[i]->id][t] = p[i]->status;
		}	
	}
	
	if(processes_completed(p, num_processes) == 1) {
		return t;
	} else {
		return pc;
	} 
	

}

