/*
* OS Scheduling Policy Simulator - Main File
* Author: Ana Berthel
* Date: 12/13/18
* 
* This program is designed to simulate various scheduling policies on workloads in a single-processor system.
* For more information, see the README file.
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<unistd.h>
#include "helper.h"

//parameter object stores simulation parameters specified from the command line
struct parameters {
	int min_CPU; //minimum CPU time a process needs
	int max_CPU; //maximum CPU time a process needs
	int min_enter; //minimum time at which a process can enter the system
	int max_enter; //maximum time at which a process can enter the system
	int io_freq; //amount of time spent making io requests 
	//If this value is 50, then a process will spend 50% as much time blocked as it does running
	int tix_gen; //strategy used to assign tickets
};

/* CONSTANTS */
static const int FIFO = 1;
static const int RR = 2;
static const int LOTTERY = 3;
static const int FB = 4;
//NOTE: Add more policies

/* METHODS */
void gen_process(struct process *proc, int i, struct parameters param, int num_processes);
void read_process(struct process *proc[], FILE *file, int num_processes);

/* MAIN METHOD */
int main(int argc, char* argv[]) {
	
	printf("*** OS Scheduling Policy Simulator ***\n");
	printf(" ** Latest release: Dec 13, 2018 **\n");
	printf("  * Author: Ana Berthel *\n\n");
	
	/* SET DEFAULT PARAMETERS */	
	int policy=FIFO;
	int num_processes = 10; //constant
	struct parameters param = {10, 100, 0, 100, 50, 0};
	
	FILE *file; //prepare file to read process list
  
  	int counter = 0; //keeps track of the arguments that have been processed
  	int process_file = FALSE; //has a file been specified?
	
  
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
    	} else if(!strcmp(arg, "-p")) { //policy used
    		counter ++;
    		char *temp = argv[counter];
    		 
    		if(strcmp(temp, "fifo") == 0) {
    		 	policy = FIFO;
    		} else if(strcmp(temp, "rr") == 0) {
    		 	policy = RR;
    		} else if(strcmp(temp, "lottery") == 0) {
    		 	policy = LOTTERY;
    		} else if(strcmp(temp, "fb") == 0) {
    		 	policy = FB;
    		} /* NOTE: Add more policies here */
    	} else if(strcmp(arg, "-tix") == 0) { //ticket assignment strategy
    		counter++;
    		check_digit(argv[counter]);
     		int temp = atoi(argv[counter]);
     		param.tix_gen = temp; // 0 for random, 1 for io, 2 for prioritizing short jobs
    	} else if(!access(arg, R_OK) && counter > 0) { //if a valid filepath is encountered
    		
    		//open file
    		file = fopen(arg, "r");
    		process_file = TRUE;
    	
    		//First line of file should be "process_num: %d" where %d is the number of processes described
    		char buffer[255];
    		fgets(buffer, 255, file);
    	
    		char *string;
    		string = strtok(buffer, " ");
    		
    		//check first line formatting
    		if(strcmp(string, "process_num:") != 0) {
    			printf("Error: improperly formatted process file \n");
    			exit(1);
    		}
    	
    		//if formatting is correct, get number of processes
    		string = strtok(NULL, " \n");
    		check_digit(string);
    		int temp = atoi(string);
    		num_processes = temp;
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
	
	if(!process_file) { //no process file has been specified: randomly generate processes
  		for(int i=0; i<num_processes; i++) {
    		p[i] = malloc(sizeof(struct process));
    		gen_process(p[i], i, param, num_processes);
  		}
  	} else { //processes have been specified by file
  		read_process(p, file, num_processes); 
  	}
  	
	file = NULL;

	/* PRINT PROCESS LIST */
	printf("Processes to run:\n\n");
	print_process_status(p, num_processes);
	

	/* SIMULATE POLICY */
	switch(policy) {
		case FIFO:
			printf("Using: First In, First Out Policy\n");
			printf("...\n");
			fifo(p, num_processes);
			break;
		case RR:
			printf("Using: Round Robin Policy\n");
			printf("...\n");
			rr(p, num_processes);
			break;
		case LOTTERY:
			printf("Using: Lottery Scheduling Policy\n");
			printf("...\n");
			lottery(p, num_processes);
			break;
		case FB:
			printf("Using: Foreground-Background Policy\n");
			printf("...\n");
			fb(p, num_processes);
			break;
		/* NOTE: Add more policies here */
		default:
			printf("Couldn't find a policy to use.\n");
	}
	
	/* FREE ALLOCATED MEMORY */
	for(int i=0; i<num_processes; i++) {
		free(p[i]->io_times);
		free(p[i]);
		p[i] = NULL;
	}
	
}

/* Reads process information from a text file to create the list of processes to simulate */
void read_process(struct process *proc[], FILE *file, int num_processes) {
	
	char buffer[255];
  	char *string; 		
  	fgets(buffer, 255, file);
  	string = strtok(buffer, " ");
  	
  	/* READ INDIVIDUAL PROCESSES */
  	for(int i=0; i<num_processes; i++) {
  		
  		//allocate memory for a new process
  		proc[i] = malloc(sizeof(struct process));
  		
  		//read to next process description
  		while(strcmp(string, "Process") != 0) {
  			fgets(buffer, 255, file);
  			string = strtok(buffer, " ");
  		}
  		
  		//process ID
		proc[i]->id = i;
  		
  		//read process information
  		for(int j=0; j<4; j++) {
  			fgets(buffer, 255, file);
  			string = strtok(buffer, " ");
  			string = strtok(NULL, " \n");
  			check_digit(string);
  			int temp = atoi(string);
  				
  			switch(j) {
  				case 0: //enter time
  					proc[i]->enter_time = temp;
  					break;
  				case 1: //CPU time
  					proc[i]->CPU_time = temp;
  					break;
  				case 2: //number of tickets assigned
  					proc[i]->tix = temp;
  					break;
  				case 3: //io frequency
  					proc[i]->num_io = temp;
  					break;
  			}
  		}
  		
  		//get IO times
  		fgets(buffer, 255, file);
  		string = strtok(buffer, " ");
  			
  		int* pointer = malloc(sizeof(int[proc[i]->num_io]));
  			
  		for(int j=0; j<proc[i]->num_io; j++) {
    		string = strtok(NULL, " \n");
    		check_digit(string); 			
    		int x = atoi(string);
      		pointer[j] = x;
    	}

    	proc[i]->io_times = pointer;
    		
    	//set defaults
    	proc[i]->time_counter = 0;
    	proc[i]->status = 0;
    	proc[i]->io_timer = 0;
	
  	} 
  	
  	//close file
  	fclose(file); 
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


