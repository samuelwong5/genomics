CC = /opt/gcc-4.9.2/bin/g++
CFLAGS = -std=c++11 -O2 -Wall -fopenmp
BB_CPP_FILES := $(wildcard src/bitbuffer/*.cpp)
MD_CPP_FILES := $(wildcard src/metadata/*.cpp)
QS_CPP_FILES := $(wildcard src/quality/*.cpp)
SQ_CPP_FILES := $(wildcard src/sequence/*.cpp)
BB_OBJ_FILES := $(addprefix bin/bitbuffer/,$(notdir $(BB_CPP_FILES:.cpp=.o))) 
MD_OBJ_FILES := $(addprefix bin/metadata/,$(notdir $(MD_CPP_FILES:.cpp=.o))) 
QS_OBJ_FILES := $(addprefix bin/quality/,$(notdir $(QS_CPP_FILES:.cpp=.o)))
SQ_OBJ_FILES := $(addprefix bin/sequence/,$(notdir $(SQ_CPP_FILES:.cpp=.o)))

all: compress

bin/quality/%.o: src/quality/%.cpp src/quality/%.hpp
	mkdir -p bin/quality
	${CC} ${CFLAGS} -c -o $@ $<
    
bin/bitbuffer/%.o: src/bitbuffer/%.cpp src/bitbuffer/%.hpp
	mkdir -p bin
	mkdir -p bin/bitbuffer
	${CC} ${CFLAGS} -c -o $@ $<

bin/metadata/%.o: src/metadata/%.cpp src/metadata/%.hpp
	mkdir -p bin/metadata
	${CC} ${CFLAGS} -c -o $@ $<

bin/sequence/%.o: src/sequence/%.cpp src/sequence/%.hpp
	mkdir -p bin/sequence
	${CC} ${CFLAGS} -c -o $@ $<

bin/main.o: src/main.cpp
	${CC} ${CFLAGS} -c -o $@ $<

compress: $(BB_OBJ_FILES) $(MD_OBJ_FILES) $(QS_OBJ_FILES) $(SQ_OBJ_FILES) bin/main.o
	$(CC) $(CFLAGS) -g $^ -o $@

clean:
	rm -rf bin/*

.PHONY: clean
