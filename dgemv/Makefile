CC = gcc-8
CFLAGS = -O1 -std=c99 -Wall -Wextra
UNLOOP_BLOCK_SIZE = 2 4
BLOCKING_WIDTH = 4 8 12 16 20 24 28 32

.PHONY: execute_all
execute_all:
	@make build_all
	@echo
	@echo ***start***
	@echo naive.o:; ./naive.o
	@echo embarassingly_parallel.o:; ./embarassingly_parallel.o
	@echo
	@echo ***end***

.PHONY: build_all
build_all:
	@make naive.o
	@make embarassingly_parallel.o

embarassingly_parallel.o: embarrassingly_parallel.c util.o
	$(CC) $(CFALGS) -fopenmp embarrassingly_parallel.c util.o -o embarassingly_parallel.o

naive.o: naive.c util.o
	$(CC) $(CFLAGS) naive.c util.o -o naive.o

util.o: util.c
	$(CC) $(CFLAGS) -c util.c

.PHONY: clean
clean:
	$(RM) ./*.o
	$(RM) -rf ./*.dSYM

.PHONY: format
format:
	clang-format $(wildcard *.c) -i
