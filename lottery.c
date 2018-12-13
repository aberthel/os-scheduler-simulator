/*
* OS Scheduling Policy Simulator - Lottery Scheduling Implementation
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
	int ttix;
	struct process* current;
};

//methods
struct results lottery_process(struct process *p[], int* results_array[], struct process *c, struct queue *blocked, int pc, int start_time, int time_estimate, int time, int num_processes, int ttix);
struct process *hold_lottery(struct process *p[], int num_processes, int total_tix);

/* Main Lottery method. Sets up the Lottery simulation. */
void lottery(struct process *p[], int num_processes) {
	
	//get time estimate	
	int time_estimate = estimate_time(p, num_processes);
	
	//make array to store process running data
	int* results_array[num_processes];
	for(int i=0; i<num_processes; i++) {
		results_array[i] = calloc(time_estimate, sizeof(int));
	}
	
	//sort processes by enter time
	selection_sort(p, num_processes);
	
	//initialize blocked process queue
	struct queue *blocked =calloc(1, sizeof(struct queue*));
	
	//keeps track of the process currently being run
	int pc = 0; 
	int start_time = 0;
	int end_time = time_estimate;
	int timer = time_slice; 
	struct process* current = NULL;	
	int total_tix = 0; 
	
	//run first round of Lottery simulation
	struct results r = lottery_process(p, results_array, current, blocked, pc, start_time, end_time, timer, num_processes, total_tix);
	
	//if first round does not result in all processes being completed, run additional rounds
	while(processes_completed(p, num_processes) == 0) {
		//increase time estimate
		start_time += time_estimate;
		end_time += time_estimate;
			
		//increase size of results array
		for(int i=0; i<num_processes; i++) {
			results_array[i] = realloc(results_array[i], sizeof(int)*end_time);
		}
		
		//run next round of Lottery simulation
		r = lottery_process(p, results_array, r.current, blocked, r.pc, start_time, end_time, r.timer, num_processes, r.ttix);
	} 
	
	//write results array to file		
	print_array_to_file(results_array, r.t, "lottery.csv", num_processes);

	//calculate metrics
	printf("Simulation complete!\n\n");
	calculate_metrics(results_array, p, r.t, num_processes);

	//free allocated memory
	for(int i=0; i<num_processes; i++) {
		free(results_array[i]);
		results_array[i] = NULL;
	}  
	free(blocked);
	blocked = NULL;
}

/* Auxilliary Lottery Method. Contains main Lottery logic */
struct results lottery_process(struct process *p[], int* results_array[], struct process *c, struct queue *blocked, int pc, int start_time, int time_estimate, int time, int num_processes, int ttix) {
	int t;
	int timer = time;
	struct process *current = c;
	int total_tix = ttix;
	
	//for each time in time estimate, determine status of each process
	for(t=start_time; t<time_estimate; t++) {
		
		//if all processes are complete, no need to go further
		if(processes_completed(p, num_processes) == 1) {
			break;
		}
		
		//place newly entered processes in the ready list and set status to 1
		while(pc < num_processes && p[pc]->enter_time <= t) {
			if(p[pc]->status == 0) {
				p[pc]->status = 1;
				total_tix = total_tix + p[pc]->tix;
			}
			pc ++;
		}
		
		
		//if no process is currently running, hold a lottery to get running process
		if(current == NULL) {
			current = hold_lottery(p, num_processes, total_tix);
			timer=time_slice;	
		} 
		
		//if current process makes IO request, set status to blocked and start IO timer
		if(current != NULL && is_io_time(current)) {
			current->status =2;
			current->io_timer=0;
			enqueue(current, blocked);
			
			//remove current's tickets from lottery
			total_tix = total_tix - current->tix;

			//get next process to run
			current = hold_lottery(p, num_processes, total_tix);
			timer = time_slice;
		}
		
		//if current process's CPU time is over, then set status to finished and get next process to run
		if(current != NULL && current->time_counter == current->CPU_time) {
			current->status = 4;
			//remove current's tickets from lottery
			total_tix = total_tix - current->tix;
			
			//get next process to run
			current = hold_lottery(p, num_processes, total_tix);
			timer = time_slice;
		}
		
		//if time slice is over, then hold lottery to get next process to run
		if(timer == 0) {
			current-> status = 1;
			//no need to remove tickets for this one, it's still in the lottery
			current = hold_lottery(p, num_processes, total_tix);
			timer = time_slice;					
		}
		
		 
		//if there are blocked processes
		if(blocked->front != NULL) {
			
			//remove processes that have completed IO request from blocked queue
			//and place them at the end of the ready queue
			while(blocked->front != NULL && blocked->front->data->io_timer == io_time) {
				struct process *a = dequeue(blocked);
				a->status = 1;
				total_tix = total_tix + a->tix;
			}
			
			//if there aren't any processes running, pick one of the just-unblocked processes to run
			if(current == NULL) {
				current = hold_lottery(p, num_processes, total_tix);
			}
			
			//increment IO timer on remaining blocked processes
			struct node* n = blocked->front;
			while(n != NULL) {
				n->data->io_timer++;
				n = n->next;
			}
			
		
		} 
		
		//increment time counter for running process
		if(current != NULL) {
			current->time_counter++;
			timer --;
		} 
		
		//save statuses at this time to file
		for(int i=0; i<num_processes; i++) {
			results_array[p[i]->id][t] = p[i]->status;
		}
	}
	
	struct results r = {pc, t, timer, total_tix, current};
	return r;
}

/* Determines which process should run next using a random number generator 
* NOTE: previous running process should be stopped BEFORE calling this function
*/
struct process *hold_lottery(struct process *p[], int num_processes, int total_tix) {
	
	//if no processes hold tickets, none are ready to be run
	if(total_tix == 0) { 
		return NULL;
	}
	
	//get random number of tickets
	int ticket = rand()%total_tix + 1;
	
	//search through ticket holders until you find the process holding the right ticket
	struct process *current = NULL;
	for(int i=0; i<num_processes; i++) {
		current = p[i];
		if(current->status == 1) { //only consider ready processes
			ticket = ticket - current->tix; //subtract tix current has from total
			
			if(ticket <= 0) { //if current has winning ticket, set it to running and return it
				current->status = 3;
				return current;
			}
		}
	}
	return NULL;
}
