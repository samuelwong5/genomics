CC = gcc
CFLAGS = -std=c11

all: bits

bits.o : bits.c bits.h
	${CC} ${CFLAGS} -c bits.c    

bits: bits.o
	${CC} -g bits.o -o bits

clean:
	rm -rf bf *.o

.PHONY: clean