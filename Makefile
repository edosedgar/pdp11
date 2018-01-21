.PHONY: all decoder pdp11
-include *.d

CXXFLAGS=-Wall -Wno-unused-function -Wno-unused-variable  -g -I$(INCLUDES)

INCLUDES=./include

CXX=g++

all: decoder pdp11

decoder: decoder_main.o decoder.o
	$(CXX) $(CXXFLAGS) $^ -o decoder

pdp11: main.o pdp11.o decoder.o memory.o
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm *.o 
