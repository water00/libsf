
CXX = g++
CC = gcc
CPPFLAGS =
CXXFLAGS = -g
CXXFLAGS += -std=c++11
CXXFLAGS += -I ../src
LDFLAGS = -g
LDFLAGS += -lpthread -ldl 

all: MultiTask

MultiTask: taskChkMain.o \
	task.o 
	$(CXX) -o $@ $^ $(LDFLAGS) 

task.o: task.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $^

taskChkMain.o: taskChkMain.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	@rm -rf *.o
	@rm -rf MultiTask

