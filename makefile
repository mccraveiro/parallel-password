CC=gcc

all: brute_force parallel


gen_hash: gen_hash.o
	$(CC) -o gen_hash gen_hash.c -lcrypt -pthread

brute_force: brute_force.o
	$(CC) -o brute_force brute_force.c -lcrypt -pthread

parallel: parallel.o
	$(CC) -o parallel parallel.c -lcrypt -pthread

clean:
	$(RM) *.o gen_hash brute_force parallel
