CFLAGS=-I.. -g

test: test-read

test-read: ../scheme
	../scheme < test-read.scm | diff - test-read.out

test-symbols: ../symbols.o test-symbols.o
	$(CC) $(CFLAGS) -o $@ ../symbols.o test-symbols.o

