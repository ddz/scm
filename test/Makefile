CFLAGS=-I.. -g

test: test-read test-eval lotsa-lists test-map test-symbols

test-read: ../scheme
	../scheme < test-read.scm | diff - test-read.out

test-eval: ../scheme
	../scheme < test-eval.scm | diff - test-eval.out

lotsa-lists: ../scheme
	time ../scheme < lotsa-lists.scm >/dev/null

test-map: ../map.o test-map.o
	$(CC) $(CFLAGS) -o $@ ../map.o test-map.o
	time ./test-map

test-symbols: ../symbols.o ../map.o test-symbols.o
	$(CC) $(CFLAGS) -o $@ ../symbols.o ../map.o test-symbols.o
	time ./test-symbols

clean:
	rm *.o *~ core *.core test-symbols test-map
