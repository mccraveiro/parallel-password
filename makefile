CC=gcc

all: gen_hash brute_force parallel

gen_hash: gen_hash.o
	$(CC) -o gen_hash gen_hash.c

brute_force: brute_force.o
	$(CC) -o brute_force brute_force.c

parallel: parallel.o
	$(CC) -o parallel parallel.c

clean:
	$(RM) *.o gen_hash brute_force parallel
