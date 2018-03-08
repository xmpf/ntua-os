CC='gcc'
CFLAGS='-Wall'
LIBS='-lc'
ECLEAN='main.o zing2.o prog prog2'

main.o : main.c
	$(CC) $< -c

zing2.o : zing2.c
	$(CC) $< -c

prog : main.o zing.o
	$(CC) $^ $(CFLAGS) -o $@ $(LIBS)

prog2 : main.o zing2.o
	$(CC) $^ $(CFLAGS) -o $@ $(LIBS)

clean :
	rm -rf $(ECLEAN)