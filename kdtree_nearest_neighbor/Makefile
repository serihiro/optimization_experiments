CC := gcc
CFLAGS := -O3 -march=native -std=c11 -Wall -Wextra -Wpedantic
LDFLAGS := -lm

TARGET := benchmark
SRC := benchmark.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
