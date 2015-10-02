CC=gcc

all: gen_hash parallel

gen_hash: gen_hash.o
	$(CC) -o gen_hash gen_hash.c -lcrypto

parallel: parallel.o
	$(CC) -o parallel parallel.c -lcrypto -g -Wall -fopenmp

clean:
	$(RM) *.o gen_hash parallel
