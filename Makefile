#
# $Id$
#

CFLAGS=-I. -g
LDLIBS=-lreadline -lcurses -ll
OBJS=scheme.o read.o lexer.o write.o types.o stk.o strbuf.o symbols.o symtable.o

all: scheme

test: scheme
	cd test && $(MAKE)

scheme: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)

scheme.c: scheme.h
	$(CC) $(CFLAGS) -c scheme.c

read.c: scheme.h lexer.h stk.h strbuf.h

lexer.c: lexer.h lex.yy.c

lex.yy.c: scheme.l
	flex -i -olex.yy.c scheme.l

write.c: scheme.h

types.c: scheme.h

stk.c: stk.h

strbuf.c: strbuf.h

symbols.c: scheme.h

symtable.c: scheme.h symtable.h

clean:
	rm *.o lex.yy.c scheme core *.core *~
