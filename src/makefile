CC = gcc
CFLAGS = -std=c11

all: bits rl

bits.o : bits.c bits.h
	${CC} ${CFLAGS} -c bits.c    

lzw.o : lzw.c bits.h
	${CC} ${CFLAGS} -c lzw.c    

rl.o : rl.c
	${CC} ${CFLAGS} -c rl.c

rl : bits.o rl.o
	${CC} -g rl.o bits.o -o rl

bits: bits.o lzw.o
	${CC} -g lzw.o bits.o -o lzw

clean:
	rm -rf bf *.o

.PHONY: clean