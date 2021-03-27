
CXX = g++
CC = gcc
CPPFLAGS =
CXXFLAGS = -g
CXXFLAGS += -std=c++11
CXXFLAGS += -I ../src
LDFLAGS = -g
LDFLAGS += -lpthread -ldl 

all: StateChk

StateChk: stateCheckMain.o \
			stateTask.o
	$(CXX) -o $@ $^ $(LDFLAGS) 

stateCheckMain.o: stateCheckMain.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $^

stateTask.o: stateTask.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	@rm -rf *.o
	@rm -rf StateChk

