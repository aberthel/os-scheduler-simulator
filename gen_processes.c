/*
* OS Scheduling Policy Simulator - Process File Generator
* Author: Ana Berthel
* Date: 12/13/18
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<unistd.h>
#include "helper.h"

//stores parameters used for generating processes
struct parameters {
	int min_CPU;
	int max_CPU;
	int min_enter;
	int max_enter;
	int io_freq;
	int tix_gen;
};

char* file_name;

//methods
void gen_process(struct process *proc, int i, struct parameters param, int num_processes);
void write_processes(struct process *p[], int num_processes);

/* MAIN METHOD */
int main(int argc, char* argv[]) {
	file_name = "processes.txt";
	
	//set default parameters
	int num_processes = 10; 
	struct parameters param = {10, 100, 0, 100, 50, 0};
	
  	int counter = 0; //keeps track of the arguments that have been processed
  
  	/* READ USER INPUT */
  	while(counter < argc) {
    
    	//get argument
    	char* arg = argv[counter];
    
    	//determine argument type
    	if(!strcmp(arg, "-np")) { //number of processes
			counter++;
     		int temp = atoi(argv[counter]);
			check_arg(temp);
			num_processes = temp;
    	} else if(!strcmp(arg, "-minCPU")) { //minimum CPU time needed for a single process
        	counter++;
     		int temp = atoi(argv[counter]);
			check_arg(temp);
			param.min_CPU = temp;
    	} else if(!strcmp(arg, "-maxCPU")) { //maximum CPU time needed for a single process
        	counter++;
     		int temp = atoi(argv[counter]);
			check_arg(temp);
			param.max_CPU = temp;
    	} else if(!strcmp(arg, "-mine")) { //earliest time at which a process can enter the system
        	counter++;
			check_digit(argv[counter]);
     		int temp = atoi(argv[counter]);
			param.min_enter = temp;
    	} else if(!strcmp(arg, "-maxe")) { //latest time at which a process can enter the system
        	counter++;
			check_digit(argv[counter]);
     		int temp = atoi(argv[counter]);
			param.max_enter = temp;
    	} else if(!strcmp(arg, "-io")) { //amount of io time each process needs (as % of CPU time)
        	counter++;
			check_digit(argv[counter]);
     		int temp = atoi(argv[counter]);
			param.io_freq = temp;
    	} else if(strcmp(arg, "-tix") == 0) { //ticket assignment strategy
    		counter++;
    		check_digit(argv[counter]);
     		int temp = atoi(argv[counter]);
     		param.tix_gen = temp; // 0 for random, 1 for io, 2 for prioritizing short jobs
    	}else { //output file name specified
    		if(counter != 0) {
    			file_name = argv[counter];
    		}
    	}
    	counter++;
  	}

	//check for logically impossible parameters
	if(param.min_CPU > param.max_CPU || param.min_enter > param.max_enter) {
		printf("Error: invalid arguments\n");
		exit(1);
	}
	
	/* GENERATE PROCESS LIST */
  	struct process *p[num_processes];
	
	//randomly generate processes
  	for(int i=0; i<num_processes; i++) {
    	p[i] = malloc(sizeof(struct process));
    	gen_process(p[i], i, param, num_processes);
  	}
  	
	/* PRINT PROCESS LIST */
	printf("Processes generated:\n\n");
	print_process_status(p, num_processes);
	
	//write process list to file
	write_processes(p, num_processes);
	
	/* FREE ALLOCATED MEMORY */
	for(int i=0; i<num_processes; i++) {
		free(p[i]->io_times);
		free(p[i]);
		p[i] = NULL;
	}
	
}


/* Randomly generate a list of processes to simulate from user parameters */
void gen_process(struct process *proc, int i, struct parameters param, int num_processes) {
	proc->id = i;
  	
  	//set defaults
  	proc->status = 0;
  	proc->io_timer = 0;
  	proc->time_counter = 0;
  	
  	//set enter time
    if(param.max_enter == param.min_enter) {
      	proc->enter_time = param.max_enter;
    } else {
      	proc->enter_time = (rand()%(param.max_enter-param.min_enter))+ param.min_enter;
    }

	//set CPU time
    if(param.max_CPU == param.min_CPU) {
   	   	proc->CPU_time = param.min_CPU;
    } else {
      	proc->CPU_time = (rand()%(param.max_CPU-param.min_CPU))+ param.min_CPU;
    }

	//set number of I/O requests
    int num_io = proc->CPU_time/io_time;
    num_io = num_io*param.io_freq/100;
    proc->num_io = num_io;
    
    
    //set number of tickets according to ticket generation strategy
    if(param.tix_gen == 0) { //random
    	int x = rand()%100;
    	proc->tix = x;
    } else if(param.tix_gen == 1) { //scales linearly with io
    	int x = proc->num_io * 10 + 10; //keeps no io processes from having 0 tickets
    	proc->tix = x;
    } else if(param.tix_gen == 2) { //scales inversely with CPU time, with some randomness
    	int x = param.max_CPU - proc->CPU_time + 5;
    	int div = param.max_CPU/20; //restricts to +/- 5% of maximum
    	
    	if(rand()%2) { //add
    		x = x + rand()%div;
    	} else { //subtract
    		x = x-rand()%div;
    		if(x < 1) { //estimate should not make tickets less than 1
    			x = 5;
    		}
    	}
    	proc->tix = x;
    }
 
    //set io times
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

/* writes process list to file */
void write_processes(struct process *p[], int num_processes) {
	printf("Writing to file: %s\n", file_name);
	FILE *file;
	file = fopen(file_name, "w");
	
	fprintf(file, "process_num: %d\n\n", num_processes);
	
	for(int i=0; i<num_processes; i++) {
		struct process *proc = p[i];
		fprintf(file, "Process %d\n", i);
		fprintf(file, "Enter %d\n", proc->enter_time);
		fprintf(file, "CPU %d\n", proc->CPU_time);
		fprintf(file, "Tickets %d\n", proc->tix);
		fprintf(file, "NumIO %d\n", proc->num_io);
		fprintf(file, "IO");
		for(int j=0; j<proc->num_io; j++) {
			fprintf(file, " %d", proc->io_times[j]);
		}
		
		fprintf(file, "\n\n");
	} 
	
	fclose(file);
}
