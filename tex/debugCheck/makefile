
CXX = g++
CC = gcc
CPPFLAGS =
CXXFLAGS = -g
CXXFLAGS += -std=c++11
CXXFLAGS += -I ../src
LDFLAGS = -g
LDFLAGS += -lpthread -ldl 

all: DebugChk

DebugChk: debugCheckMain.o
	$(CXX) -o $@ $^ $(LDFLAGS) 

debugCheckMain.o: debugCheckMain.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	@rm -rf *.o
	@rm -rf DebugChk

