#
# $Id$
#

CFLAGS=-I. -g
LDLIBS=-lgmp
OBJS=main.o read.o write.o types.o stk.o strbuf.o pairs.o symbols.o numbers.o numberio.o symtable.o

all: scheme

test: scheme
	cd test && $(MAKE)

scheme: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)

read.c: scheme.h stk.h strbuf.h

write.c: scheme.h

types.c: scheme.h

stk.c: stk.h

strbuf.c: strbuf.h

pairs.c: scheme.h

symbols.c: scheme.h

symtable.c: scheme.h symtable.h

clean:
	rm *.o lex.yy.c scheme core *.core *~
