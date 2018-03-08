.PHONY: all clean

all: ask3-signals

CC = gcc
CFLAGS = -g -Wall -O2

ask3-signals: ask3-signals.o proc-common.o tree.o
	$(CC) $(CFLAGS) $^ -o $@

%.s: %.c
	$(CC) $(CFLAGS) -S -fverbose-asm $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.i: %.c
	gcc -Wall -E $< | indent -kr > $@

clean: 
	rm -f *.o ask3-signals
