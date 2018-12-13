#ifndef PROC_QUEUE_
#define PROC_QUEUE_

/* process structures store crucial information about each process */
struct process {
  int id; //process number
  int enter_time; //time at which process enters the system
  int CPU_time; //total CPU time needed
  int* io_times; //array of times at which process makes io request
  int num_io; //total number of io requests needed
  int time_counter; //keeps track of how much CPU time has been used
  int tix; //for priority/lottery scheduling
  int status; //not entered, ready, blocked, running, or finished
  int io_timer; //keeps track of time spent in an io request
};
  
  
//constants
static const int io_time = 10;
static const int time_slice = 10;

//helper functions
int isInArray(int x, int* arr, int until);
void check_digit(char* s);
void check_arg(int t);
int estimate_time(struct process *p[], int num_processes);
void selection_sort(struct process *p[], int num_processes);
void print_array_to_file(int* array[], int l, char* file_name, int num_processes);
void print_process_status(struct process *p[], int np);
int is_io_time(struct process *x);
int processes_completed(struct process *p[], int num_processes);
void calculate_metrics(int* results[], struct process* p[], int length, int num_processes);
void calculate_metrics_groups(int* results[], struct process* p[], int length, int num_processes, int sep);

/* PROCESS QUEUE */

struct node {
	struct process *data;
	struct node *next;
};

struct queue {
	struct node *front;
	struct node *back;
};

/* functions */
void enqueue(struct process *p, struct queue *q);
struct node* make_new_node(struct process *p);
void print_queue(struct queue *q);
struct process* dequeue(struct queue *q);
void move_to_back(struct queue *q);



/*FIFO */

void fifo(struct process *p[], int num_processes);


/* ROUND ROBIN */

void rr(struct process *p[], int num_processes);

/* LOTTERY */
void lottery(struct process *p[], int num_processes);

/* FOREGROUND BACKGROUND */
void fb(struct process *p[], int num_processes);

#endif