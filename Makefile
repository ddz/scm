#
# $Id$
#

CC=cc
CFLAGS=-g -Wall -pedantic
LDLIBS=-lreadline -ll

all: scheme

scheme: scheme.o read.o lexer.o write.o types.o stk.o strbuf.o

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
	rm *.o lex.yy.c read core *.core *~
