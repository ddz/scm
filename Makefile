#
# $Id$
#

CFLAGS=-g -Wall -pedantic
LDLIBS=-lreadline -lcurses -ll

all: scheme

test: scheme
	./scheme < test/test-read.scm | diff - ./test/test-read.out

scheme: scheme.o read.o lexer.o write.o types.o stk.o strbuf.o
	$(CC) $(CFLAGS) -o $@ $? $(LDFLAGS) $(LDLIBS)

scheme.c: scheme.h

read.c: scheme.h lexer.h stk.h strbuf.h

lexer.c: lexer.h lex.yy.c

lex.yy.c: scheme.l
	flex scheme.l

write.c: scheme.h

types.c: scheme.h

stk.c: stk.h

strbuf.c: strbuf.h

clean:
	rm *.o lex.yy.c scheme core *.core *~
