.PHONY: all clean

all: ask4-pipes ask4-pipes-v2

CC = gcc
CFLAGS = -g -Wall -O2
SHELL= /bin/bash

ask4-pipes: ask4-pipes.o proc-common.o tree.o
	$(CC) $(CFLAGS) $^ -o $@

ask4-pipes-v2: ask4-pipes-v2.c proc-common.o tree.o
	$(CC) $(CFLAGS) $^ -o $@

%.s: %.c
	$(CC) $(CFLAGS) -S -fverbose-asm $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.i: %.c
	gcc -Wall -E $< | indent -kr > $@

clean: 
	rm -f *.o ask4-pipes ask4-pipes-v2
