CXX=g++
CXXFLAGS=-Wall -std=c++17 -DDEBUG
EXE_NAME=injx.exe

SRCS=injx.cc
OBJS=$(subst .cc,.o,$(SRCS))

all: injx

injx: $(OBJS)
	$(CXX) $(CXXFLAGS) $(DEFS) -o $(EXE_NAME) $(OBJS)

clean:
	rm -f injx.o injx.exe

.PHONY: all clean

