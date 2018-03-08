CC='gcc'
CFLAGS='-Wall'
LIBS='-lc'

fconc.o : fconc.c
	$(CC) $< -c


fconc : fconc.o
	$(CC) $^ $(CFLAGS) -o $@ $(LIBS)

clean:
	rm -rf *.o fconc out.txt
