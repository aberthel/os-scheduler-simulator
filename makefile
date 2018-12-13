all: TBD.c
	gcc -Wall -o PolSim TBD.c queue_func.c helper_func.c fifo.c rr.c lottery.c fb.c

gen: gen_processes.c
	gcc -Wall -o gen_p gen_processes.c helper_func.c 

debug: TBD.c
	gcc -Wall -g -o PolSim_debug TBD.c queue_func.c helper_func.c fifo.c rr.c lottery.c fb.c

clean:
	rm -f PolSim PolSim_debug
