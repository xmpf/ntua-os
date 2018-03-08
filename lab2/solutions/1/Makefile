.PHONY: all clean

all: ex2_1

CC = gcc
CFLAGS = -g -Wall -O2
SHELL= /bin/bash

ex2_1: ex2_1.o proc-common.o tree.o
	$(CC) $(CFLAGS) $^ -o $@

%.s: %.c
	$(CC) $(CFLAGS) -S -fverbose-asm $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.i: %.c
	gcc -Wall -E $< | indent -kr > $@

clean: 
	rm -f *.o ex2_1
