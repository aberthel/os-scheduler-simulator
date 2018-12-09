#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include "proc_queue.h"

/* FIRST IN FIRST OUT */

int fifo_process(struct process *p[], int* results_array[], int pc, int time_start, int time_estimate, int num_processes);

void fifo(struct process *p[], int num_processes) {
	int time_estimate = estimate_time(p, num_processes);
	
	//make array to store process running data
	int* results_array[num_processes];
	
	for(int i=0; i<num_processes; i++) {
		results_array[i] = calloc(time_estimate, sizeof(int));
	}
	
	//sort processes by enter time
	selection_sort(p, num_processes);

	
	//keeps track of the process currently being run
	int pc = 0;
	int start_time = 0;
	int end_time = time_estimate;

	pc = fifo_process(p, results_array, pc, start_time, end_time, num_processes);
	
	
	//handles "time over"
	while(processes_completed(p, num_processes) == 0) {
		printf("NOT FINISHED\n");
		start_time += time_estimate;
		end_time += time_estimate;
			
		//increase size of results array
		for(int i=0; i<num_processes; i++) {
			results_array[i] = realloc(results_array[i], sizeof(int)*end_time);
		}
		pc = fifo_process(p, results_array, pc, start_time, end_time, num_processes);
	}
		
	
	print_array_to_file(results_array, pc, "fifo.csv", num_processes);

	printf("Made it out alive!\n");

	calculate_metrics(results_array, p, pc, num_processes);
	
	calculate_metrics_groups(results_array, p, pc, num_processes, 80);

	//TODO: remember to free allocated memory!
	for(int i=0; i<num_processes; i++) {
		free(results_array[i]);
		results_array[i] = NULL;
	}  
}


int fifo_process(struct process *p[], int* results_array[], int pc, int time_start, int time_estimate, int num_processes) {
	int t;
	for(t=time_start; t<time_estimate; t++) {
		if(processes_completed(p, num_processes) == 1) {
			break;
		}
		
		//place newly entered processes in the ready list
		int counter = pc;
		
		while(counter < num_processes && p[counter]->enter_time <= t) {
		
			if(p[counter]->status == 0) {
				p[counter]->status = 1;
			}
			counter ++;
		}
		
		//set next process's status to running if it is running
		if(p[pc]->status == 1) {
			p[pc]->status = 3;
		}
		
		//set process status to blocked if time for io
		if(p[pc]->status ==3 && is_io_time(p[pc])) {
			p[pc]->status =2;
			p[pc]->io_timer=0;
		}
		
		//if io is finished, then set status to running again
		if(p[pc]->status == 2 && p[pc]->io_timer == io_time) {
			p[pc]->status = 3;
		}
		
		//if CPU time is over, then set status to finished and increment counter
		if(p[pc]->status == 3 && p[pc]->time_counter == p[pc]->CPU_time) {
			p[pc]->status = 4;
			pc ++;
			if(p[pc]->status == 1) {
				p[pc]->status = 3;
			}
		}
		
		//increment time counters
		if(p[pc]->status == 3) {
			p[pc]->time_counter++;
		}
		
		if(p[pc]->status == 2) {
			p[pc]->io_timer ++;
		}
		
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

