# Makefile for C_progs ----- James Arram 2014

CXX=/opt/gcc-4.9.2/bin/g++
CXXFLAGS=-Wall -O3 -fopenmp -g -Wextra -std=c++11
LFLAGS=

SRCDIR=src
OBJDIR=obj
SRCS=$(wildcard $(SRCDIR)/*.cpp)
DEPS=$(wildcard $(SRCDIR)/*.hpp)
OBJS=$(patsubst $(SRCDIR)/%.cpp, obj/%.o, $(SRCS))

all: compress

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	@ mkdir -p obj
	$(CXX) $< $(CXXFLAGS) -c -o $@

compress: $(OBJS) $(DEPS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

run:
	./compress /mnt/ssd2/jma11/index/oversample/3-step/hg38.fmt /mnt/ssd2/jma11/reads/ERR161544_1.fastq
