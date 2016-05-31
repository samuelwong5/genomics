CC = g++
CFLAGS = -std=c++11 -O2 -Wall

all: qualityscoreencoder

qualityscoreencoder: qualityscoreencoder.o bitbuffer.o
	${CC} -g qualityscoreencoder.o bitbuffer.o -o qualityscoreencoder
    
qualityscoreencoder.o: qualityscoreencoder.cpp
	${CC} ${CFLAGS} -c qualityscoreencoder.cpp
    
bitbuffer.o: bitbuffer.cpp bitbuffer.hpp
	${CC} ${CFLAGS} -c bitbuffer.cpp  

clean:
	rm -rf bf *.o

.PHONY: clean