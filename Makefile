#
# $Id$
#

CC=cc
CFLAGS=-g
LDLIBS=-lreadline -ll

all: read

read: read.o

read.o: lex.yy.c tokens.h types.h

lex.yy.c: r5rs.f
	$(LEX) r5rs.f

clean:
	$(RM) *.o lex.yy.c
