CC ?= cc
CFLAGS ?= -O3 -march=native -std=c11
LDFLAGS ?=

all: naive optimized

naive: naive.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

optimized: optimized.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f naive optimized

.PHONY: all clean
