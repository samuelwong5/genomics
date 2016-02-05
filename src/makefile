CC = g++
CFLAGS = -std=c++11 -O2 -Wall

all: rl_encoder lzw_encoder

lzw_encoder: lzw_encoder.o bitbuffer.o
	${CC} -g lzw_encoder.o bitbuffer.o -o lzw_encoder
    
lzw_encoder.o: lzw_encoder.cpp lzw_encoder.hpp
	${CC} ${CFLAGS} -c lzw_encoder.cpp

rl_encoder: rl_encoder.o bitbuffer.o
	${CC} -g rl_encoder.o bitbuffer.o -o rl_encoder
    
rl_encoder.o: rl_encoder.cpp rl_encoder.hpp
	${CC} ${CFLAGS} -c rl_encoder.cpp
    
bitbuffer.o: bitbuffer.cpp bitbuffer.hpp
	${CC} ${CFLAGS} -c bitbuffer.cpp
   
clean:
	rm -rf bf *.o

.PHONY: clean