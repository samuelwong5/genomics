CC = gcc
CFLAGS = -std=c11

all: benchmark

bits.o : bits.c bits.h
	${CC} ${CFLAGS} -c bits.c    

lzw.o : lzw.c lzw.h bits.h
	${CC} ${CFLAGS} -c lzw.c    

rl.o : rl.c rl.h bits.h
	${CC} ${CFLAGS} -c rl.c

naive_rl.o : naive_rl.c naive_rl.h bits.h
	${CC} ${CFLAGS} -c naive_rl.c    

huffman.o : huffman.c huffman.h bits.h
	${CC} ${CFLAGS} -c huffman.c    

benchmark.o: benchmark.c rl.h lzw.h naive_rl.h huffman.h
	${CC} ${CFLAGS} -c benchmark.c

benchmark: bits.o lzw.o rl.o benchmark.o naive_rl.o huffman.o
	${CC} -g benchmark.o bits.o lzw.o naive_rl.o rl.o huffman.o -o benchmark

clean:
	rm -rf bf *.o

.PHONY: clean