#
# $Id$
#

CC=cc
CFLAGS=-g
LDLIBS=-lreadline -ll

all: read

read: read.o write.o types.o stk.o que.o

read.o: scheme.h lex.yy.c

lex.yy.c: scheme.l
	flex scheme.l

write.o: scheme.h

types.o: types.c types.h

scheme.h: types.h _types.h

clean:
	$(RM) *.o
