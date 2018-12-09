#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include "proc_queue.h"

/* LOTTERY */


/* returns relevant data for processing */
struct results {
	int pc;
	int t;
	int timer;
	int ttix;
	struct process* current;
};

struct results lottery_process(struct process *p[], int* results_array[], struct process *c, struct queue *blocked, int pc, int start_time, int time_estimate, int time, int num_processes, int ttix);
struct process *hold_lottery(struct process *p[], int num_processes, int total_tix);

void lottery(struct process *p[], int num_processes) {
	int time_estimate = estimate_time(p, num_processes);
	
	//make array to store process running data
	int* results_array[num_processes];
	
	for(int i=0; i<num_processes; i++) {
		results_array[i] = calloc(time_estimate, sizeof(int));
	}
	
	//this makes it easier to determine what to add onto the queue next
	selection_sort(p, num_processes);
	
	struct queue *blocked =calloc(1, sizeof(struct queue*));
	
	int pc = 0; //keeps track of the processes that have entered
	int start_time = 0;
	int end_time = time_estimate;
	int timer = time_slice; //counts down time slice
	
	//int t;
	struct process* current = NULL;
	
	int total_tix = 0; //NEEDS TO BE PRESERVED
	

	struct results r = lottery_process(p, results_array, current, blocked, pc, start_time, end_time, timer, num_processes, total_tix);
	
	
	//handles "time over"
	while(processes_completed(p, num_processes) == 0) {
		printf("NOT FINISHED\n");
		start_time += time_estimate;
		end_time += time_estimate;
			
		//increase size of results array
		for(int i=0; i<num_processes; i++) {
			results_array[i] = realloc(results_array[i], sizeof(int)*end_time);
		}
		r = lottery_process(p, results_array, r.current, blocked, r.pc, start_time, end_time, r.timer, num_processes, r.ttix);
	} 
		
	print_array_to_file(results_array, r.t, "lottery.csv", num_processes);

	printf("Made it out alive!\n");

	calculate_metrics(results_array, p, r.t, num_processes);
	
	//calculate_metrics_groups(results_array, p, r.t, num_processes, 80);

	//TODO: remember to free allocated memory!
	for(int i=0; i<num_processes; i++) {
		free(results_array[i]);
		results_array[i] = NULL;
	}  
}

struct results lottery_process(struct process *p[], int* results_array[], struct process *c, struct queue *blocked, int pc, int start_time, int time_estimate, int time, int num_processes, int ttix) {
	int t;
	int timer = time;
	struct process *current = c;
	int total_tix = ttix;
	
	/*
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
	
		
	} */
	
	
	
	for(t=start_time; t<time_estimate; t++) {
		
		if(processes_completed(p, num_processes) == 1) {
			break;
		}
		
		//place newly entered processes in the ready list
		while(pc < num_processes && p[pc]->enter_time <= t) {
			if(p[pc]->status == 0) {
				p[pc]->status = 1;
				total_tix = total_tix + p[pc]->tix;
			}
			pc ++;
		}
		
		
		//if there's no process running currently, then get one from the queue
		if(current == NULL) {
			current = hold_lottery(p, num_processes, total_tix); ///NEED TO DEFINE A LOTTERY FUNCTION
			timer=time_slice;	
		} 
		
		//if current issues an io request
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
		
		//if CPU time is over, then set status to finished and increment counter
		if(current != NULL && current->time_counter == current->CPU_time) {
			current->status = 4;
			//remove current's tickets from lottery
			total_tix = total_tix - current->tix;
			
			//get next process to run
			current = hold_lottery(p, num_processes, total_tix);
			timer = time_slice;
		}
		
		//if time slice is over, then put current running status to back
		if(timer == 0) {
			//printf("Time slice over!\n");
			current-> status = 1;
			//no need to remove tickets for this one, it's still in the lottery
			
			current = hold_lottery(p, num_processes, total_tix);
			timer = time_slice;
								
		}
		
		 
		//deal with io stuff
		if(blocked->front != NULL) {
			
			while(blocked->front != NULL && blocked->front->data->io_timer == io_time) {
				//blocked->front->data->status = 1;
				struct process *a = dequeue(blocked);
				a->status = 1;
				total_tix = total_tix + a->tix;
			}
			
			//if there aren't any processes running, pick one of the just-unblocked processes to run
			if(current == NULL) {
				current = hold_lottery(p, num_processes, total_tix);
			}
		
			struct node* n = blocked->front;
			
			//decrement io time left on remaining blocked processes
			while(n != NULL) {
				n->data->io_timer++;
				n = n->next;
			}
			
		
		} 
		
		
		//increment time counters
		if(current != NULL) {
			current->time_counter++;
			//printf("Process %d: time %d\n", current->id, current->time_counter);
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

//determines which process should run next
//NOTE: need to stop previous process running BEFORE calling this one
struct process *hold_lottery(struct process *p[], int num_processes, int total_tix) {
	//printf("Holding a lottery!\n");
	//printf("Total Tickets: %d\n", total_tix);
	
	if(total_tix == 0) { //no processes hold tickets, means none have entered
		return NULL;
	}
	
	//get random number of tickets
	int ticket = rand()%total_tix + 1;
	
	//printf("Ticket chosen: %d\n", ticket);
	
	struct process *current = NULL;
	//search through ticket holders until you find the process holding the right ticket
	for(int i=0; i<num_processes; i++) {
		current = p[i];
		if(current->status == 1) { //only consider ready processes
			//printf("Process %d has %d tickets\n", current->id, current->tix);
			ticket = ticket - current->tix; //subtract tix current has from total
			if(ticket <= 0) { //if current has winning ticket, set it to running and return it
				//printf("Process %d chosen\n\n", current->id);
				current->status = 3;
				return current;
			}
		}
	
	}
	
	return NULL;
}
