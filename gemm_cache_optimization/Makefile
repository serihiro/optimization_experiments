CC ?= cc
OPT_LEVEL ?= -O3
CFLAGS ?= $(OPT_LEVEL) -march=native -std=c11
LDFLAGS ?=

all: naive optimized

naive: naive.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

optimized: optimized.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f naive optimized

.PHONY: all clean
