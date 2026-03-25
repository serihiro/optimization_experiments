CC=gcc
CFLAGS=-O3 -std=c11 -Wall -Wextra -march=native -mavx2 -mfma

.PHONY: all run clean

all: benchmark.o

benchmark.o: benchmark.c
	$(CC) $(CFLAGS) benchmark.c -o benchmark.o

run: benchmark.o
	./benchmark.o

clean:
	$(RM) benchmark.o
