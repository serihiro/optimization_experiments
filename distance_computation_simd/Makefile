CC ?= gcc
OPT_LEVEL ?= -O3
CFLAGS ?= $(OPT_LEVEL) -std=c11 -Wall -Wextra -march=native -mavx2 -mfma

.PHONY: all run clean

all: benchmark.o

benchmark.o: benchmark.c
	$(CC) $(CFLAGS) benchmark.c -lm -o benchmark.o

run: benchmark.o
	./benchmark.o

clean:
	$(RM) benchmark.o
