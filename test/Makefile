CFLAGS=-I.. -g

test: test-symbols

test-read: ../scheme
	../scheme < test-read.scm | diff - test-read.out

test-symbols: ../symbols.o ../symtable.o test-symbols.o
	$(CC) $(CFLAGS) -o $@ ../symbols.o ../symtable.o test-symbols.o

