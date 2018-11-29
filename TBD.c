#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<unistd.h>

/* process structures store crucial information about each process */
struct process {
  int id;
  int enter_time;
  int CPU_time;
  int* io_times;
  int num_io;
  int time_counter;
  int priority;
  int status;
  int io_timer;
};

//constants
int io_time = 10;

//things to be specified
int num_processes = 10;
/* PARAMETER DEFAULTS */  
int min_CPU_time = 10;
int max_CPU_time = 100;
int min_enter_time = 0;
int max_enter_time = 100;
int io_freq = 50; // ~50% of the time should be spent in io
	
//methods
void print_process_status(struct process *p[], int np);
int isInArray(int x, int* arr, int until);
void check_digit(char* s);
void fifo(struct process *p[]);
int estimate_time(struct process *p[]);
int comp_times(const void *a, const void *b);
int is_io_time(struct process *x);
int processes_completed(struct process *p[]);
void print_array_to_file(int* array[], int l, char* file_name);
void check_arg(int t);
void gen_process(struct process *proc, int i);
void read_process(struct process *proc[], FILE *file);
void selection_sort(struct process *p[]);
int fifo_process(struct process *p[], int* results_array[], int pc, int time_start, int time_estimate);

/* main method */
int main(int argc, char* argv[]) {


	/* SET PROCESS PARAMETERS */
	FILE *file;
  
  	int counter = 0; //keeps track of the arguments that have been processed
  	int process_file = 0; //0 if no process file has been specified, 1 otherwise

  
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
			min_CPU_time = temp;
    	} else if(strcmp(arg, "-maxCPU") == 0) {
        	counter++;
     		int temp = atoi(argv[counter]);
			check_arg(temp);
			max_CPU_time = temp;
    	} else if(strcmp(arg, "-mine") == 0) {
        	counter++;
			check_digit(argv[counter]);
     		int temp = atoi(argv[counter]);
			min_enter_time = temp;
    	} else if(strcmp(arg, "-maxe") == 0) {
        	counter++;
			check_digit(argv[counter]);
     		int temp = atoi(argv[counter]);
			max_enter_time = temp;
    	} else if(strcmp(arg, "-io") == 0) {
        	counter++;
			check_digit(argv[counter]);
     		int temp = atoi(argv[counter]);
			io_freq = temp;
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
    		gen_process(p[i], i);
  		}
  	} else { //processes have been specified by file
  		read_process(p, file);
  	}
  	
  	file = NULL;
	
	fifo(p);
	//print_process_status(p, num_processes);
}

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


void read_process(struct process *proc[], FILE *file) {
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
  					proc[i]->priority = temp;
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


void gen_process(struct process *proc, int i) {
	proc->id = i;
  	proc->status = 0;
  	proc->io_timer = 0;
  	
  	
    if(max_enter_time == min_enter_time) {
      	proc->enter_time = max_enter_time;
    } else {
      	proc->enter_time = (rand()%(max_enter_time-min_enter_time))+ min_enter_time;
    }

    if(max_CPU_time == min_CPU_time) {
   	   	proc->CPU_time = min_CPU_time;
    } else {
      	proc->CPU_time = (rand()%(max_CPU_time-min_CPU_time))+ min_CPU_time;
    }

	
    proc->time_counter = 0;
    proc->priority = 50;
	
    int num_io = proc->CPU_time/io_time;
    num_io = num_io*io_freq/100;

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


//sorts the process by enter time
//because I can't get qsort to work
void selection_sort(struct process *p[]) {
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




/* FIRST IN FIRST OUT */


void fifo(struct process *p[]) {
	int time_estimate = estimate_time(p);
	
	//make array to store process running data
	int* results_array[num_processes];
	
	for(int i=0; i<num_processes; i++) {
		results_array[i] = calloc(time_estimate, sizeof(int));
	}
	
	//sort processes by enter time
	//qsort(p, num_processes, sizeof(struct process*), comp_times);
	selection_sort(p);


	//print_process_status(p, num_processes);

	
	//keeps track of the process currently being run
	int pc = 0;
	int start_time = 0;
	int end_time = time_estimate;

	pc = fifo_process(p, results_array, pc, start_time, end_time);
	
	
	//handles "time over"
	while(processes_completed(p) == 0) {
		printf("NOT FINISHED\n");
		start_time += time_estimate;
		end_time += time_estimate;
			
		//increase size of results array
		for(int i=0; i<num_processes; i++) {
			results_array[i] = realloc(results_array[i], sizeof(int)*end_time);
		}
		pc = fifo_process(p, results_array, pc, start_time, end_time);
	}
		
	
	
	
	print_array_to_file(results_array, pc, "fifo.csv");

	printf("Made it out alive!\n");


	//TODO: remember to free allocated memory!
	for(int i=0; i<num_processes; i++) {
		free(results_array[i]);
		results_array[i] = NULL;
	}  
}


int fifo_process(struct process *p[], int* results_array[], int pc, int time_start, int time_estimate) {
	int t;
	for(t=time_start; t<time_estimate; t++) {
		if(processes_completed(p) == 1) {
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
	
	if(processes_completed(p) == 1) {
		return t;
	} else {
		return pc;
	}

}



void print_array_to_file(int* array[], int l, char* file_name) {
	FILE *file;
	file = fopen(file_name, "w");
	
	for(int i=0; i<num_processes; i++) {
		for(int j=0; j<l; j++) {
			fprintf(file, "%d,", array[i][j]);
		}
		
		fprintf(file, "\n");
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


int processes_completed(struct process *p[]) {
	for(int i=0; i<num_processes; i++) {
		if(p[i]->time_counter < p[i]->CPU_time) {
			return 0;
		}
	}
	return 1;
}


int comp_times(const void *a, const void *b) {
	return (*(struct process*)a).enter_time - (*(struct process*)b).enter_time; 
}

int estimate_time(struct process *p[]) {
	int time_estimate = 0;
	
	for(int i=0; i<num_processes; i++) {
		time_estimate += p[i]->CPU_time;
		time_estimate += p[i]->num_io * io_time;
	}
	
	return time_estimate;
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
