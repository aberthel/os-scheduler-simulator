all: TBD.c
	gcc -Wall -o TBD TBD.c queue_func.c helper_func.c fifo.c rr.c

debug: TBD.c
	gcc -Wall -g -o TDB_debug TBD.c

clean:
	rm -f TBD TBD_debug
