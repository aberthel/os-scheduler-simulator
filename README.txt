README

OS Scheduling Policy Simulator
Author: Ana Berthel
Latest update: 12/13/18

This program can be used to simulate single-processor scheduling policies on various workloads. Current supported policies are First In First Out, Round Robin, Lottery, and Foreground Background. 

Installation:
Run "make all" to compile the program on your system.

To Run:
The main executable is named PolSim by default, and can be run from the command line. Several flags are used to tailor the parameters of the simulation to an individual user's needs. The flags specify the policy to be used, and they guide the generation of processes on which to test the provided policy. For example, the user can specify the number of processes to be generated, the earliest and latest times at which a process can enter the system, and the frequency of I/O requests. Flags may be included in any order. A list of available flags is included below.

Flag		Allowed Values			Use
-p		fifo, rr, lottery, fb		Policy to simulate
-np		Any positive integer		Number of processes to generate
-minCPU		Any positive integer		minimum CPU time required for a single process
-maxCPU		Any positive integer		maximum CPU time required for a single process
-mine		Any non-negative integer	earliest time at which a process can enter the system
-maxe		Any non-negative integer	latest time at which a process can enter the system
-io		Any non-negative integer	Amount of time a process should spend making I/O requests, as a percentage of the total CPU time
-tix		0, 1, 2				Ticket assignment strategy for Lottery scheduling: 0 assigns tickets randomly, 1 assigns tickets in proportion to the number of I/O requests made, 2 assigns tickets in inverse proportion to job length

For example, the following would be a valid set of inputs for this program. In this example, the user wishes to simulate a Round Robin scheduling policy. The simulation will run on 15 processes, which require between 50 and 500 CPU cycles  of runtime each. They all enter the system at time 0, and will be blocked waiting for I/O requests for about 30% as long as they will be running. 

./PolSim -np 15 -minCPU 50 -maxCPU 500 -mine 0 -maxe 0 -io 30 -p rr


Alternatively, a path to a process file can be included in order to avoid random generation of processes. This can be used to allow for replicability across simulation sessions, or to avoid gratuitous use of flags. Process files can be generated using the program PGen (which is compiled at the same time as PolSim when make all is called) using the same set of flags as the main program. Process files can also be written manually. Process files must begin with the total number of processes contained in the file. Processes are then listed sequentially. An example of the formatting is shown below. See /workloads for more examples.

process_num: 3

Process 0
Enter 0
CPU 20
Tickets 49
NumIO 3
IO 44 6 41

Process 1
Enter 10
CPU 10
Tickets 44
NumIO 1
IO 5

Process 2
Enter 5
CPU 15
Tickets 50
NumIO 0
IO



The simulation will provide a summary of the processes to be run, and policy to be used. Then, it will begin the simulation. This simulation is non-interactive. When the simulation completes, average performance metrics are calculated for each process and displayed on the console. Currently, the program calculates turnaround time, wait time, and response time. 
Additionally, a .csv file is generated containing the results of the simulation. The .csv file contains a two dimensional array of the status of each process during each CPU cycle. Each row represents a process, and each column represents a CPU cycle in increasing order. Statuses are represented by integers: a key is below. 

Int	Status
0	Not yet entered1	Ready2	Blocked3	Running4	Finished

Any questions about this simulator may be directed to Ana Berthel at berth22a@mtholyoke.edu. 
