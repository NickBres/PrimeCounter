CC = gcc
CFLAGS = -std=gnu11

.PHONY: all
all: randomGenerator primeCounter myCounter

randomGenerator:  generator.c
	$(CC)  -o randomGenerator generator.c

primeCounter:	primeCounter.c
	$(CC)  -o primeCounter primeCounter.c

myCounter: MyCounter.c MyCounter.h
	$(CC) $(CFLAGS)  -o myCounter MyCounter.c

.PHONY: clean
clean:
	-rm randomGenerator primeCounter myCounter 2>/dev/null
	-find . -name "*.dSYM" -type d -exec rm -rf {} +
	clear

rerun:
	make clean
	make
	clear
