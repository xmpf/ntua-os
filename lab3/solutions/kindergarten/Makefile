#
# Makefile
# 

CC = gcc

# CAUTION: Always use '-pthread' when compiling POSIX threads-based
# applications, instead of linking with "-lpthread" directly.
CFLAGS = -Wall -O2 -pthread
LIBS = 

all: kgarten kgarten-2

## Kindergarten
kgarten: kgarten.o
	$(CC) $(CFLAGS) -o kgarten kgarten.o $(LIBS)

kgarten.o: kgarten.c
	$(CC) $(CFLAGS) -c -o kgarten.o kgarten.c

kgarten-2: kgarten-2.o
	$(CC) $(CFLAGS) -o kgarten-2 kgarten-2.o $(LIBS)

kgarten-2.o: kgarten-2.c
	$(CC) $(CFLAGS) -c -o kgarten-2.o kgarten-2.c

clean:
	rm -f *.s *.o kgarten kgarten-2 
