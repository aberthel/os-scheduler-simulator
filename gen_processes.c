#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<unistd.h>
#include "proc_queue.h"

/* stores parameters used for generating processes */
struct parameters {
	int min_CPU;
	int max_CPU;
	int min_enter;
	int max_enter;
	int io_freq;
};

char* file_name;

//methods
void gen_process(struct process *proc, int i, struct parameters param, int num_processes);
void write_processes(struct process *p[], int num_processes);

/* main method */
int main(int argc, char* argv[]) {
	file_name = "untitled.txt";
	int num_processes = 10; //constant
  
  	int counter = 0; //keeps track of the arguments that have been processed

	//set default parameter values 
	struct parameters param = {10, 100, 0, 100, 50};
	
  
  //loop through arguments to parse them
  	while(counter < argc) {
    
    	// get argument
    	char* arg = argv[counter];
    
    	if(strcmp(arg, "-np")==0) {
			counter++;
     		int temp = atoi(argv[counter]);
			check_arg(temp);
			num_processes = temp;
    	} else if(strcmp(arg, "-minCPU") == 0) {
        	//first three commands here could be collapsed into one...
        	counter++;
     		int temp = atoi(argv[counter]);
			check_arg(temp);
			param.min_CPU = temp;
    	} else if(strcmp(arg, "-maxCPU") == 0) {
        	counter++;
     		int temp = atoi(argv[counter]);
			check_arg(temp);
			param.max_CPU = temp;
    	} else if(strcmp(arg, "-mine") == 0) {
        	counter++;
			check_digit(argv[counter]);
     		int temp = atoi(argv[counter]);
			param.min_enter = temp;
    	} else if(strcmp(arg, "-maxe") == 0) {
        	counter++;
			check_digit(argv[counter]);
     		int temp = atoi(argv[counter]);
			param.max_enter = temp;
    	} else if(strcmp(arg, "-io") == 0) {
        	counter++;
			check_digit(argv[counter]);
     		int temp = atoi(argv[counter]);
			param.io_freq = temp;
    	} else {
    		if(counter != 0) {
    			file_name = argv[counter];
    		}
    	}
    	counter++;
  	}


  //create processes based off of the list given
  struct process *p[num_processes];

	//no process file has been specified: randomly generate processes

  	for(int i=0; i<num_processes; i++) {
    	p[i] = malloc(sizeof(struct process));
    	gen_process(p[i], i, param, num_processes);
  	}
  	
	
	printf("Processes to run:\n\n");
	print_process_status(p, num_processes);
	
	write_processes(p, num_processes);
	
}


void gen_process(struct process *proc, int i, struct parameters param, int num_processes) {
	proc->id = i;
  	proc->status = 0;
  	proc->io_timer = 0;
  	
  	
    if(param.max_enter == param.min_enter) {
      	proc->enter_time = param.max_enter;
    } else {
      	proc->enter_time = (rand()%(param.max_enter-param.min_enter))+ param.min_enter;
    }

    if(param.max_CPU == param.min_CPU) {
   	   	proc->CPU_time = param.min_CPU;
    } else {
      	proc->CPU_time = (rand()%(param.max_CPU-param.min_CPU))+ param.min_CPU;
    }

	
    proc->time_counter = 0;
    proc->priority = 50;
	
    int num_io = proc->CPU_time/io_time;
    num_io = num_io*param.io_freq/100;

    proc->num_io = num_io;
    
    int* pointer = malloc(sizeof(int[num_io]));

    for(int j=0; j<num_io; j++) {
    	int x = (rand()%proc->CPU_time);
    	while(isInArray(x, pointer, j-1) == 0) {
    		x = (rand()%proc->CPU_time);
    	}
      	pointer[j] = x;
    }

    proc->io_times = pointer;

}

void write_processes(struct process *p[], int num_processes) {
	printf("%s\n", file_name);
	FILE *file;
	file = fopen(file_name, "w");
	
	fprintf(file, "process_num: %d\n\n", num_processes);
	
	for(int i=0; i<num_processes; i++) {
		struct process *proc = p[i];
		fprintf(file, "Process %d\n", i);
		fprintf(file, "Enter %d\n", proc->enter_time);
		fprintf(file, "CPU %d\n", proc->CPU_time);
		fprintf(file, "Priority %d\n", proc->priority);
		fprintf(file, "NumIO %d\n", proc->num_io);
		fprintf(file, "IO");
		for(int j=0; j<proc->num_io; j++) {
			fprintf(file, " %d", proc->io_times[j]);
		}
		
		fprintf(file, "\n\n");
	} 
	
	fclose(file);
}
