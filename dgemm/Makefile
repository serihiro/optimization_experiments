CC = gcc
CFLAGS = -O1 -std=c99 -Wall -Wextra
UNLOOP_BLOCK_SIZE = 2 4
BLOCKING_WIDTH = 4 8 12 16 20 24 28 32

.PHONY: execute_all
execute_all:
	@make build_all
	@echo
	@echo ***start***
	@echo naive.o:; ./naive.o
	@echo loop_exchange.o:; ./loop_exchange.o
	@echo padding.o:; ./padding.o
	$(foreach size, $(UNLOOP_BLOCK_SIZE), echo "loop_unloop_$(size):"; ./loop_unloop_$(size).o;)
	$(foreach width, $(BLOCKING_WIDTH), echo "blocking_$(width):"; ./blocking_$(width).o;)
	@echo
	@echo ***end***

.PHONY: build_all
build_all:
	@make naive.o
	@make loop_exchange.o
	@make padding.o
	@make unloop
	@make blocking

naive.o: naive.c util.o
	$(CC) $(CFLAGS) naive.c util.o -o naive.o

loop_exchange.o: loop_exchange.c util.o
	$(CC) $(CFLAGS) loop_exchange.c util.o -o loop_exchange.o

padding.o: padding.c util.o
	$(CC) $(CFLAGS) padding.c util.o -o padding.o

unloop: util.o
	$(foreach size, $(UNLOOP_BLOCK_SIZE), $(CC) $(CFLAGS) loop_unloop_$(size).c util.o -o loop_unloop_$(size).o;)

blocking: util.o
	$(foreach width, $(BLOCKING_WIDTH), $(CC) $(CFLAGS) blocking_$(width).c util.o -o blocking_$(width).o;)

util.o: util.c
	$(CC) $(CFLAGS) -c util.c

.PHONY: clean
clean:
	$(RM) ./*.o
	$(RM) -rf ./*.dSYM
