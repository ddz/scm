#
# $Id$
#

CC=cc
CFLAGS=-g
LDLIBS=-lreadline -ll

all: read

read: read.o write.o types.o

read.o: lex.yy.c scheme.h

write.o: scheme.h

types.o: types.c types.h

scheme.h: types.h _types.h

lex.yy.c: r5rs.f tokens.h
	$(LEX) r5rs.f

clean:
	$(RM) *.o lex.yy.c
