CFLAGS=-I.. -g

test: test-read lotsa-lists test-symbols

test-read: ../scheme
	../scheme < test-read.scm | diff - test-read.out

lotsa-lists: ../scheme
	time ../scheme < lotsa-lists.scm >/dev/null

test-symbols: ../symbols.o ../symtable.o test-symbols.o
	$(CC) $(CFLAGS) -o $@ ../symbols.o ../symtable.o test-symbols.o

