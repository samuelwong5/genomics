# Makefile for C_progs ----- James Arram 2014

CXX=g++
INCL=libdivsufsort-master/build/include
LIBS=libdivsufsort-master/build/lib
CXXFLAGS=-Wall -O3 -fopenmp -g -Wextra -D_FILE_OFFSET_BITS=64 -I$(INCL) -L$(LIBS)
LFLAGS=-ldivsufsort64

SRCDIR=src
OBJDIR=obj
SRCS=$(wildcard $(SRCDIR)/*.cpp)
DEPS=$(wildcard $(SRCDIR)/*.hpp)
OBJS=$(patsubst $(SRCDIR)/%.cpp, obj/%.o, $(SRCS))

all: index_build

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	@ mkdir -p obj
	$(CXX) $< $(CXXFLAGS) -c -o $@

index_build: $(OBJS) $(DEPS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

clean:
	rm -rf obj libdivsufsort-master index_build