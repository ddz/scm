#
# $Id$
#

CC=cc
CFLAGS=-g -Wall -pedantic
LDLIBS=-lreadline -ll

all: read

read: read.o write.o types.o stk.o strbuf.o lexer.o

read.o: types.h _types.h lex.yy.c

lexer.o: lexer.c lexer.h lex.yy.c

lex.yy.c: scheme.l
	flex scheme.l

write.o: scheme.h

types.o: types.c types.h

clean:
	rm *.o lex.yy.c read core *.core *~
