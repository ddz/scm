#
# $Id$
#

CFLAGS=-I. -g
LDLIBS=-lgmp
OBJS=main.o gc.o read.o write.o types.o stk.o strbuf.o pairs.o symbols.o numbers.o numberio.o map.o env.o procedures.o cpseval.o ports.o

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

procedures.c: scheme.h env.h procedures.h

clean:
	rm *.o lex.yy.c scheme core *.core *~
