all: pol_sim.c queue_func.c helper_func.c fifo.c rr.c lottery.c fb.c gen_processes.c 
	gcc -Wall -o PolSim pol_sim.c queue_func.c helper_func.c fifo.c rr.c lottery.c fb.c
	gcc -Wall -o PGen gen_processes.c helper_func.c 

sim: pol_sim.c queue_func.c helper_func.c fifo.c rr.c lottery.c fb.c
	gcc -Wall -o PolSim TBD.c queue_func.c helper_func.c fifo.c rr.c lottery.c fb.c

gen: gen_processes.c helper_func.c
	gcc -Wall -o PGen gen_processes.c helper_func.c 

debug: pol_sim.c queue_func.c helper_func.c fifo.c rr.c lottery.c fb.c
	gcc -Wall -g -o PolSim_debug TBD.c queue_func.c helper_func.c fifo.c rr.c lottery.c fb.c

clean:
	rm -f PolSim PolSim_debug PGen *.csv
