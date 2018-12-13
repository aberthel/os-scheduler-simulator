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
	
//methods
void gen_process(struct process *proc, int i, struct parameters param, int num_processes);
void read_process(struct process *proc[], FILE *file, int num_processes);

/* main method */
int main(int argc, char* argv[]) {
	int policy=1; //1 for fifo, 2 for rr, 3 for lottery, 4 for fb
	int num_processes = 10; //constant
	
	/* SET PROCESS PARAMETERS */
	FILE *file;
  
  	int counter = 0; //keeps track of the arguments that have been processed
  	int process_file = 0; //0 if no process file has been specified, 1 otherwise

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
    	} else if(strcmp(arg, "-p") == 0) {
    		counter ++;
    		char *temp = argv[counter];
    		 
    		if(strcmp(temp, "fifo") == 0) {
    		 	policy = 1;
    		} else if(strcmp(temp, "rr") == 0) {
    		 	policy = 2;
    		} else if(strcmp(temp, "lottery") == 0) {
    		 	policy = 3;
    		} else if(strcmp(temp, "fb") == 0) {
    		 	policy = 4;
    		}
    	} else if(access(arg, R_OK) == 0 && counter != 0) { //filepath with process file
    		file = fopen(arg, "r");
    		process_file = 1;
    	
    		//First line of file should be "process_num: %d" where %d is the number of processes described
    		char buffer[255];
    		fgets(buffer, 255, file);
    	
    		char *string;
    		string = strtok(buffer, " ");
    	
    		if(strcmp(string, "process_num:") != 0) {
    			printf("Error: improperly formatted process file \n");
    			exit(1);
    		}
    	
    		string = strtok(NULL, " \n");
    		check_digit(string);
    	
    		int temp = atoi(string);
    		num_processes = temp;
    	}
    	counter++;
  	}


  	//create processes based off of the list given
  	struct process *p[num_processes];

	//no process file has been specified: randomly generate processes
	if(process_file == 0) {
  		for(int i=0; i<num_processes; i++) {
    		p[i] = malloc(sizeof(struct process));
    		gen_process(p[i], i, param, num_processes);
  		}
  	} else { //processes have been specified by file
  		read_process(p, file, num_processes); 
  	}
  	

  	file = NULL;

	printf("Processes to run:\n\n");
	//print_process_status(p, num_processes);
	
	//RUN THE POLICY
	
	switch(policy) {
		case 1:
			printf("First In, First Out Policy\n");
			fifo(p, num_processes);
			break;
		case 2:
			printf("Round Robin Policy\n");
			rr(p, num_processes);
			break;
		case 3:
			printf("Lottery Scheduling Policy\n");
			lottery(p, num_processes);
			break;
		case 4:
			printf("Foreground-Background Policy\n");
			fb(p, num_processes);
			break;
		default:
			printf("Couldn't find a policy to use.\n");
	}
	

	for(int i=0; i<num_processes; i++) {
		free(p[i]->io_times);
		free(p[i]);
		p[i] = NULL;
	}
	
}

void read_process(struct process *proc[], FILE *file, int num_processes) {
	
	char buffer[255];
  	char *string;
  		
  	fgets(buffer, 255, file);
  	string = strtok(buffer, " ");
  	
  	for(int i=0; i<num_processes; i++) {
  		
  		proc[i] = malloc(sizeof(struct process));
  		
  		//get to next process description
  		while(strcmp(string, "Process") != 0) {
  			fgets(buffer, 255, file);
  			string = strtok(buffer, " ");
  		}
  			
		proc[i]->id = i;
  			
  		for(int j=0; j<4; j++) {
  			fgets(buffer, 255, file);
  			string = strtok(buffer, " ");
  			string = strtok(NULL, " \n");
  			check_digit(string);
  			int temp = atoi(string);
  				
  			switch(j) {
  				case 0: 
  					proc[i]->enter_time = temp;
  					break;
  				case 1:
  					proc[i]->CPU_time = temp;
  					break;
  				case 2:
  					proc[i]->tix = temp;
  					break;
  				case 3:
  					proc[i]->num_io = temp;
  					break;
  			}
  		}
  		
  		//get line with IO numbers on it
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
    		
    		proc[i]->time_counter = 0;
    		proc[i]->status = 0;
    		proc[i]->io_timer = 0;
	
  	} 
  	fclose(file); 
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
    //TODO @@@@@@@@@@@@@@@@@@@@@@@@@@ Implement different policies for assigning tickets
    proc->tix = rand()%100 + 1;
	
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



