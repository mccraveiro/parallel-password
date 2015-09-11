CC=gcc

all: gen_hash parallel

gen_hash: gen_hash.o
	$(CC) -o gen_hash gen_hash.c -lcrypt

parallel: parallel.o
	$(CC) -o parallel parallel.c -lcrypt -pthread

clean:
	$(RM) *.o gen_hash brute_force parallel
