all: TBD.c
	gcc -Wall -o TBD TBD.c

debug: TBD.c
	gcc -Wall -g -o TDB_debug TBD.c

clean:
	rm -f TBD TBD_debug
