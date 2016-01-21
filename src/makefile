CC = gcc
CFLAGS = -std=c11

all: benchmark

bits.o : bits.c bits.h
	${CC} ${CFLAGS} -c bits.c    

lzw.o : lzw.c bits.h
	${CC} ${CFLAGS} -c lzw.c    

rl.o : rl.c
	${CC} ${CFLAGS} -c rl.c

benchmark.o: benchmark.c rl.h lzw.h
	${CC} ${CFLAGS} -c benchmark.c

benchmark: bits.o lzw.o rl.o benchmark.o
	${CC} -g benchmark.o bits.o lzw.o rl.o -o benchmark

clean:
	rm -rf bf *.o

.PHONY: clean