/*
* OS Scheduling Policy Simulator - Accessory Functions
* Author: Ana Berthel
* Date: 12/13/18
*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "helper.h"

/* Returns true if the given value is in the array, false otherwise */
int isInArray(int x, int* arr, int until) {
	for(int i=0; i<until; i++) {
		if(x == arr[i]) {
			return FALSE;
		}
	}
	return TRUE;
}

/* Returns a pessimistic estimate of the time required to run the processes in the list.
* Estimate is the sum of all CPU times and IO times needed by all processes.
*/
int estimate_time(struct process *p[], int num_processes) {
	int time_estimate = 0;
	
	for(int i=0; i<num_processes; i++) {
		time_estimate += p[i]->CPU_time;
		time_estimate += p[i]->num_io * io_time;
	}
	
	return time_estimate;
}

/* Sorts array of process pointers in order of increasing enter time */
void selection_sort(struct process *p[], int num_processes) {
	struct process *temp;
	int min;
	
	for(int i=0; i<num_processes; i++) {
		min=i;
		for(int j=i+1; j<num_processes; j++) {
			if(p[j]->enter_time < p[min]->enter_time) {
				min=j;
			}
		}
		
		//swap index i and min
		if(i != min) {
			temp = p[i];
			p[i] = p[min];
			p[min] = temp;
		}
		
		
	}

}

/* writes the provided array to a .csv file */
void print_array_to_file(int* array[], int l, char* file_name, int num_processes) {
	FILE *file;
	file = fopen(file_name, "w");
	
	for(int i=0; i<num_processes; i++) {
		for(int j=0; j<l; j++) {
			fprintf(file, "%d,", array[i][j]);
		}
		
		fprintf(file, "\n");
	} 
	fclose(file);
}

/* Calculates average metrics for the finished simulation */
void calculate_metrics(int* results[], struct process *p[], int length, int num_processes) {
	
	int tt[num_processes]; //turnaround time
	int rt[num_processes]; //response time
	int wt[num_processes]; //wait time
	
	//for averages
	int avg_t = 0;
	int avg_r = 0;
	int avg_w = 0;
	
	for(int i=0; i<num_processes; i++) {
		int enter = -1; //time at which the process enters the system
		int first_run = -1; //time at which the process is first run
		int exit = -1; //time at which the process exits the system
		
		for(int j=0; j<length; j++) {
			if(enter==-1 && results[p[i]->id][j]>0) {
				enter = j;
			}
			if(first_run==-1 && results[p[i]->id][j] == 3) {
				first_run = j;
			}
			if(exit==-1 && results[p[i]->id][j]==4) {
				exit = j;
				break;
			}
		}
		
		//handles if process is last to run
		if(exit == -1) {
			exit = length;
		}
		
		tt[p[i]->id] = exit-enter;
		rt[p[i]->id] = first_run-enter;
		wt[p[i]->id] = (exit-enter)-p[i]->CPU_time -(io_time*p[i]->num_io);
		
		avg_t += tt[p[i]->id];
		avg_r += rt[p[i]->id];
		avg_w += wt[p[i]->id];
	}
	
	//calculate averages
	double avg_t1 = ((double) avg_t)/num_processes;
	double avg_r1 = ((double) avg_r)/num_processes;
	double avg_w1 = ((double) avg_w)/num_processes;
	
	printf("Metrics: average\n");
	printf("Turnaround time: %lf\n", avg_t1);
	printf("Wait time: %lf\n", avg_w1);
	printf("Response time: %lf\n\n", avg_r1);

}

/* Calculates average metrics for the finished simulation for two groups of processes */
void calculate_metrics_groups(int* results[], struct process *p[], int length, int num_processes, int sep) {
	
	int tt[num_processes]; //turnaround time
	int rt[num_processes]; //response time
	int wt[num_processes]; //wait time
	
	//for averages group 1
	int avg_t_1 = 0;
	int avg_r_1 = 0;
	int avg_w_1 = 0;
	
	//for averages group 2
	int avg_t_2 = 0;
	int avg_r_2 = 0;
	int avg_w_2 = 0;
	
	
	for(int i=0; i<num_processes; i++) {
		int enter = -1; //time at which the process enters the system
		int first_run = -1; //time at which the process is first run
		int exit = -1; //time at which the process exits the system
		
		for(int j=0; j<length; j++) {
			if(enter==-1 && results[p[i]->id][j]>0) {
				enter = j;
			}
			if(first_run==-1 && results[p[i]->id][j] == 3) {
				first_run = j;
			}
			if(exit==-1 && results[p[i]->id][j]==4) {
				exit = j;
				break;
			}
		}
		//handles if process is last to run
		if(exit == -1) {
			exit = length;
		}
		
		tt[p[i]->id] = exit-enter;
		rt[p[i]->id] = first_run-enter;
		wt[p[i]->id] = (exit-enter)-p[i]->CPU_time;
		
		if(p[i]->id < sep) {
			avg_t_1 += tt[p[i]->id];
			avg_r_1 += rt[p[i]->id];
			avg_w_1 += wt[p[i]->id];
		} else {
			avg_t_2 += tt[p[i]->id];
			avg_r_2 += rt[p[i]->id];
			avg_w_2 += wt[p[i]->id];
		}
	}
	
	//calculate averages
	double avg_t1 = ((double) avg_t_1)/sep;
	double avg_r1 = ((double) avg_r_1)/sep;
	double avg_w1 = ((double) avg_w_1)/sep;
	
	double avg_t2 = ((double) avg_t_2)/(num_processes-sep);
	double avg_r2 = ((double) avg_r_2)/(num_processes-sep);
	double avg_w2 = ((double) avg_w_2)/(num_processes-sep);
	
	printf("Metrics: average group 1\n");
	printf("Turnaround time: %lf\n", avg_t1);
	printf("Wait time: %lf\n", avg_w1);
	printf("Response time: %lf\n\n", avg_r1);
	
	printf("Metrics: average group 2\n");
	printf("Turnaround time: %lf\n", avg_t2);
	printf("Wait time: %lf\n", avg_w2);
	printf("Response time: %lf\n\n", avg_r2);
	
}


/* checks to make sure that the first character of an input is a digit */
void check_digit(char* s) {
	if(isdigit(s[0]) == 0) {
		printf("Error: expected positive number \n");
		exit(1);
	}
}

/* checks to make sure that a string input has been successfully converted to an integer */
void check_arg(int t) {
	if(t <= 0) {
	  	printf("Error: invalid argument input\n");
	  	exit(1);
	}
}

/* Prints basic information about each process in list */
void print_process_status(struct process *p[], int np) {
  for(int i=0; i<np; i++) {
    printf("Process %d:\n", p[i]->id);
    
    
    printf("Enters system at %d\n", p[i]->enter_time);
    printf("Total CPU time: %d\n", p[i]->CPU_time);
    printf("Number of tickets: %d\n", p[i]->tix);
   
   
    printf("I/O request times: ");
    for(int j=0; j<p[i]->num_io; j++) {
      printf("%d ", p[i]->io_times[j]);   
    }
    printf("\n\n\n");
  }
}

/* Returns true if the given process makes an IO request at this time point */
 int is_io_time(struct process *x) {
	for(int i=0; i<x->num_io; i++) {
		if(x->time_counter == x->io_times[i]) {
			return TRUE;
		}
	}
	return FALSE;
}

/* Returns true if all processes have completed, false otherwise */
int processes_completed(struct process *p[], int num_processes) {
	for(int i=0; i<num_processes; i++) {
		if(p[i]->time_counter < p[i]->CPU_time) {
			return FALSE;
		}
	}
	return TRUE;
}



