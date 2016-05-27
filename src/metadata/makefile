CC = g++
CFLAGS = -std=c++11 -O2 -Wall

all: metadataencoder

metadataencoder: metadataencoder.o metadatafieldencoder.o alphanumericfieldencoder.o autoincrementingfieldencoder.o constantalphanumericfieldencoder.o numericfieldencoder.o bitbuffer.o
	${CC} -g metadataencoder.o metadatafieldencoder.o alphanumericfieldencoder.o autoincrementingfieldencoder.o constantalphanumericfieldencoder.o numericfieldencoder.o bitbuffer.o -o metadataencoder
    
metadataencoder.o: metadataencoder.cpp
	${CC} ${CFLAGS} -c metadataencoder.cpp
    
metadatafieldencoder.o: metadatafieldencoder.cpp metadatafieldencoder.hpp
	${CC} ${CFLAGS} -c metadatafieldencoder.cpp

alphanumericfieldencoder.o: alphanumericfieldencoder.cpp alphanumericfieldencoder.hpp
	${CC} ${CFLAGS} -c alphanumericfieldencoder.cpp
    
autoincrementingfieldencoder.o: autoincrementingfieldencoder.cpp autoincrementingfieldencoder.hpp
	${CC} ${CFLAGS} -c autoincrementingfieldencoder.cpp
    
constantalphanumericfieldencoder.o: constantalphanumericfieldencoder.cpp constantalphanumericfieldencoder.hpp
	${CC} ${CFLAGS} -c constantalphanumericfieldencoder.cpp
  
numericfieldencoder.o: numericfieldencoder.cpp numericfieldencoder.hpp
	${CC} ${CFLAGS} -c numericfieldencoder.cpp  
    
bitbuffer.o: bitbuffer.cpp bitbuffer.hpp
	${CC} ${CFLAGS} -c bitbuffer.cpp  
    
clean:
	rm -rf bf *.o

.PHONY: clean