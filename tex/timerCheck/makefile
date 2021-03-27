
CXX = g++
CC = gcc
CPPFLAGS =
CXXFLAGS = -g
CXXFLAGS += -std=c++11
CXXFLAGS += -I ../src
LDFLAGS = -g
LDFLAGS += -lpthread -ldl 

all: TimerChk

TimerChk: timerCheckMain.o \
			timerTask.o
	$(CXX) -o $@ $^ $(LDFLAGS) 

timerCheckMain.o: timerCheckMain.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $^

timerTask.o: timerTask.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	@rm -rf *.o
	@rm -rf TimerChk

