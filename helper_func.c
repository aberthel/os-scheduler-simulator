#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "proc_queue.h"


int isInArray(int x, int* arr, int until) {
	for(int i=0; i<until; i++) {
		if(x == arr[i]) {
			return 0;
		}
	}
	return 1;
}

void check_digit(char* s) {
	if(isdigit(s[0]) == 0) {
		printf("Error: expected number \n");
		exit(1);
	}
}

void check_arg(int t) {
	if(t <= 0) {
	  	printf("Error: invalid argument input\n");
	  	exit(1);
	}
}

int estimate_time(struct process *p[], int num_processes) {
	int time_estimate = 0;
	
	for(int i=0; i<num_processes; i++) {
		time_estimate += p[i]->CPU_time;
		time_estimate += p[i]->num_io * io_time;
	}
	
	return time_estimate;
}


//sorts the process by enter time
//because I can't get qsort to work
void selection_sort(struct process *p[], int num_processes) {
	//temporary variable to hold 
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


void print_array_to_file(int* array[], int l, char* file_name, int num_processes) {
	FILE *file;
	file = fopen(file_name, "w");
	
	for(int i=0; i<num_processes; i++) {
		for(int j=0; j<l; j++) {
			fprintf(file, "%d,", array[i][j]);
		}
		
		fprintf(file, "\n");
	} 

}




void print_process_status(struct process *p[], int np) {
  for(int i=0; i<np; i++) {
    printf("Process %d:\n", p[i]->id);
    
    
    printf("Enters system at %d\n", p[i]->enter_time);
    printf("Total CPU time: %d\n", p[i]->CPU_time);
    printf("Priority: %d\n", p[i]->priority);
   
   
    printf("I/O request times: ");
    for(int j=0; j<p[i]->num_io; j++) {
      printf("%d ", p[i]->io_times[j]);   
    }
    printf("\n");
    
    printf("CPU time completed: %d\n", p[i]->time_counter);
    printf("\n\n"); 
  }
}

 int is_io_time(struct process *x) {
	for(int i=0; i<x->num_io; i++) {
		if(x->time_counter == x->io_times[i]) {
			return 1;
		}
	}
	return 0;
}



int processes_completed(struct process *p[], int num_processes) {
	for(int i=0; i<num_processes; i++) {
		if(p[i]->time_counter < p[i]->CPU_time) {
			return 0;
		}
	}
	return 1;
}

