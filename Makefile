#
# $Id$
#

CC=cc
CFLAGS=-g
LDLIBS=-lreadline

all: read

read: read.o write.o types.o

read.o: scheme.h

write.o: scheme.h

types.o: types.c types.h

scheme.h: types.h _types.h

clean:
	$(RM) *.o
