CC = g++
CFLAGS = -std=c++11 -O2 -Wall
BB_CPP_FILES := $(wildcard src/bitbuffer/*.cpp)
MD_CPP_FILES := $(wildcard src/metadata/*.cpp)
QS_CPP_FILES := $(wildcard src/quality/*.cpp)
BB_OBJ_FILES := $(addprefix bin/bitbuffer/,$(notdir $(BB_CPP_FILES:.cpp=.o))) 
MD_OBJ_FILES := $(addprefix bin/metadata/,$(notdir $(MD_CPP_FILES:.cpp=.o))) 
QS_OBJ_FILES := $(addprefix bin/quality/,$(notdir $(QS_CPP_FILES:.cpp=.o)))

all: qualityencoder

bin/bitbuffer/%.o: src/bitbuffer/%.cpp src/bitbuffer/%.hpp
	${CC} ${CFLAGS} -c -o $@ $<

bin/metadata/%.o: src/metadata/%.cpp src/metadata/%.hpp
	${CC} ${CFLAGS} -c -o $@ $<

bin/quality/%.o: src/quality/%.cpp src/quality/%.hpp
	${CC} ${CFLAGS} -c -o $@ $<

metadataencoder: $(BB_OBJ_FILES) $(MD_OBJ_FILES)
	$(CC) $(CFLAGS) -g $^ -o $@

qualityencoder: $(BB_OBJ_FILES) $(QS_OBJ_FILES)
	$(CC) $(CFLAGS) -g $^ -o $@

clean:
	rm -rf bf *.o

.PHONY: clean