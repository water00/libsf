
CXX = g++
CC = gcc
CPPFLAGS =
CXXFLAGS = -g
CXXFLAGS += -std=c++11
CXXFLAGS += -I ../src
LDFLAGS = -g
LDFLAGS += -lpthread -ldl 

all: MessagePong

MessagePong: taskMain.o \
	task1.o \
	task2.o 
	$(CXX) -o $@ $^ $(LDFLAGS) 

task1.o: task1.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $^

task2.o: task2.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $^

taskMain.o: taskMain.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	@rm -rf *.o
	@rm -rf MessagePong

