CC = gcc
CFLAGS = -std=c11

all: bits

bits.o : bits.c bits.h
	${CC} ${CFLAGS} -c bits.c    

lzw.o : lzw.c bits.h
	${CC} ${CFLAGS} -c lzw.c    

bits: bits.o lzw.o
	${CC} -g lzw.o bits.o -o lzw

clean:
	rm -rf bf *.o

.PHONY: clean