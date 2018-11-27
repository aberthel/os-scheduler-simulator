#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<unistd.h>

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
int num_processes;

//methods
void print_process_status(struct process p[], int np);
int isInArray(int x, int* arr, int until);
void check_digit(char* s);
void fifo(struct process p[]);
int estimate_time(struct process p[]);
int comp_times(const void *a, const void *b);
int is_io_time(struct process x);
int processes_completed(struct process p[]);
void print_array_to_file(int* array[], int l, char* file_name);

int main(int argc, char* argv[]) {
  num_processes = 10;

  int min_CPU_time = 10;
  int max_CPU_time = 100;

  int min_enter_time = 0;
  int max_enter_time = 100;

  int io_freq = 50; // 50% of the time should be spent in io

  /* SET PROCESS PARAMETERS */


  //TODO handling for parsing text files

	FILE *file;
  
  int counter = 0;
  int process_file = 0; //0 if no process list has been specified, 1 otherwise

  
  //loop through arguments to parse them
  while(counter < argc) {
    
    char* arg = argv[counter];
    
    if(strcmp(arg, "-np")==0) {
	counter++;
     	int temp = atoi(argv[counter]);
		if(temp == 0) {
	  		printf("Error: invalid argument input\n");
	  		exit(1);
	  	}
		num_processes = temp;
    } else if(strcmp(arg, "-minCPU") == 0) {
        counter++;
     	int temp = atoi(argv[counter]);

		if(temp <= 0) {
	  		printf("Error: invalid argument input\n");
	  		exit(1);
	  	}

		min_CPU_time = temp;
    } else if(strcmp(arg, "-maxCPU") == 0) {
        counter++;
     	int temp = atoi(argv[counter]);

		if(temp <= 0) {
	  		printf("Error: invalid argument input\n");
	  		exit(1);
	  	}

		max_CPU_time = temp;
    } else if(strcmp(arg, "-mine") == 0) {
        counter++;
	
		if(isdigit(argv[counter][0]) ==0) {
	  		printf("Error: invalid argument input\n");
	  		exit(1);
		}
	
     	int temp = atoi(argv[counter]);

		min_enter_time = temp;
    } else if(strcmp(arg, "-maxe") == 0) {
        counter++;

		if(isdigit(argv[counter][0]) ==0) {
	  		printf("Error: invalid argument input\n");
	  		exit(1);
		}
	
     	int temp = atoi(argv[counter]);

		max_enter_time = temp;
    } else if(strcmp(arg, "-io") == 0) {
        counter++;

		if(isdigit(argv[counter][0]) ==0) {
	  		printf("Error: invalid argument input\n");
	  		exit(1);
		}
	
     	int temp = atoi(argv[counter]);

		io_freq = temp;
    } else if(access(arg, R_OK) == 0 && counter != 0) {
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
    	
    	if(isdigit(string[0]) == 0) {
    		printf("Error: improperly formatted process file \n");
    		exit(1);
    	}
    	
    	int temp = atoi(string);
    	num_processes = temp;
    }


    counter++;
  }



  
  //create processes based off of the list given
  struct process p[num_processes];

	//no process file has been specified: randomly generate processes
	if(process_file == 0) {
  		for(int i=0; i<num_processes; i++) {
  			p[i].id = i;
  			p[i].status = 0;
  			p[i].io_timer = 0;
    		if(max_enter_time == min_enter_time) {
      			p[i].enter_time = max_enter_time;
    		} else {
      			p[i].enter_time = (rand()%(max_enter_time-min_enter_time))+ min_enter_time;
    		}

    		if(max_CPU_time == min_CPU_time) {
   	   			p[i].CPU_time = min_CPU_time;
    		} else {
      			p[i].CPU_time = (rand()%(max_CPU_time-min_CPU_time))+ min_CPU_time;
    		}


    		p[i].time_counter = 0;
    		p[i].priority = 50;

    		int num_io = p[i].CPU_time/io_time;
    		num_io = num_io*io_freq/100;

    		p[i].num_io = num_io;
    
    		int* pointer = malloc(sizeof(int[num_io]));

    		for(int j=0; j<num_io; j++) {
    			int x = (rand()%p[i].CPU_time);
    			while(isInArray(x, pointer, j-1) == 0) {
    				x = (rand()%p[i].CPU_time);
    			}
      			pointer[j] = x;
    		}

    		p[i].io_times = pointer;
  		}
  	} else { //processes have been specified by file
  		char buffer[255];
  		char *string;
  		
  		fgets(buffer, 255, file);
  		string = strtok(buffer, " ");
  		
  		for(int i=0; i<num_processes; i++) {
  			//get to next process description
  			while(strcmp(string, "Process") != 0) {
  				fgets(buffer, 255, file);
  				string = strtok(buffer, " ");
  			}
  			
			p[i].id = i;
  			
  			for(int j=0; j<4; j++) {
  				fgets(buffer, 255, file);
  				string = strtok(buffer, " ");
  				string = strtok(NULL, " \n");
  				check_digit(string);
  				int temp = atoi(string);
  				
  				switch(j) {
  					case 0: 
  						p[i].enter_time = temp;
  						break;
  					case 1:
  						p[i].CPU_time = temp;
  						break;
  					case 2:
  						p[i].priority = temp;
  						break;
  					case 3:
  						p[i].num_io = temp;
  						break;
  				}
  			}
  			
  			//get line with IO numbers on it
  			fgets(buffer, 255, file);
  			string = strtok(buffer, " ");
  			
  			int* pointer = malloc(sizeof(int[p[i].num_io]));
  			
  			for(int j=0; j<p[i].num_io; j++) {
    			string = strtok(NULL, " \n");
    			check_digit(string); 			
    			int x = atoi(string);
      			pointer[j] = x;
    		}

    		p[i].io_times = pointer;
    		
    		p[i].time_counter = 0;
    		p[i].status = 0;
    		p[i].io_timer = 0;

  		}
  		fclose(file);
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




/* FIRST IN FIRST OUT */


void fifo(struct process p[]) {
	int time_estimate = estimate_time(p);
	
	//make array to store process running data
	int* results_array[num_processes];
	
	for(int i=0; i<num_processes; i++) {
		results_array[i] = calloc(time_estimate, sizeof(int));
	}

	//sort processes by enter time
	qsort(p, num_processes, sizeof(struct process), comp_times);



	//print_process_status(p, num_processes);


	//keeps track of the total time of the system
	//int master_time = 0;
	
	//keeps track of the process currently being run
	int pc = 0;

	//TODO: this implementation does NOT handle time over
	for(int t=0; t<time_estimate; t++) {
		if(processes_completed(p) == 1) {
			break;
		}
	
		//place newly entered processes in the ready list
		int counter = pc;
		while(p[counter].enter_time <= t) {
			if(p[counter].status == 0) {
				p[counter].status = 1;
			}
			counter ++;
		}
		
		//set next process's status to running if it is running
		if(p[pc].status == 1) {
			p[pc].status = 3;
		}
		
		//set process status to blocked if time for io
		if(p[pc].status ==3 && is_io_time(p[pc])) {
			p[pc].status =2;
			p[pc].io_timer=0;
		}
		
		//if io is finished, then set status to running again
		if(p[pc].status == 2 && p[pc].io_timer == io_time) {
			p[pc].status = 3;
		}
		
		//if CPU time is over, then set status to finished and increment counter
		if(p[pc].status == 3 && p[pc].time_counter == p[pc].CPU_time) {
			p[pc].status = 4;
			pc ++;
			if(p[pc].status == 1) {
				p[pc].status = 3;
			}
		}
		
		//increment time counters
		if(p[pc].status == 3) {
			p[pc].time_counter++;
		}
		
		if(p[pc].status == 2) {
			p[pc].io_timer ++;
		}
		
		for(int i=0; i<num_processes; i++) {
			results_array[p[i].id][t] = p[i].status;
		}
	
	}
	
	print_array_to_file(results_array, time_estimate, "fifo.csv");

	printf("Made it out alive!\n");


	//TODO: remember to free allocated memory!
	for(int i=0; i<num_processes; i++) {
		free(results_array[i]);
		results_array[i] = NULL;
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




int is_io_time(struct process x) {
	for(int i=0; i<x.num_io; i++) {
		if(x.time_counter == x.io_times[i]) {
			return 1;
		}
	}
	return 0;
}


int processes_completed(struct process p[]) {
	for(int i=0; i<num_processes; i++) {
		if(p[i].time_counter < p[i].CPU_time) {
			return 0;
		}
	}
	return 1;
}


int comp_times(const void *a, const void *b) {
	return (*(struct process*)a).enter_time - (*(struct process*)b).enter_time; 
}

int estimate_time(struct process p[]) {
	int time_estimate = 0;
	
	for(int i=0; i<num_processes; i++) {
		time_estimate += p[i].CPU_time;
		time_estimate += p[i].num_io * io_time;
	}
	
	return time_estimate;
}



void print_process_status(struct process p[], int np) {
  for(int i=0; i<np; i++) {
    printf("Process %d:\n", p[i].id);
    printf("Enters system at %d\n", p[i].enter_time);
    printf("Total CPU time: %d\n", p[i].CPU_time);
    printf("Priority: %d\n", p[i].priority);
   
    printf("I/O request times: ");
    for(int j=0; j<p[i].num_io; j++) {
      printf("%d ", p[i].io_times[j]);   
    }
    printf("\n");
    
    printf("CPU time completed: %d\n", p[i].time_counter);
    printf("\n\n");
  }
}
