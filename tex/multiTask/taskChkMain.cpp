#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include "task.h"
#include "sfDebug.h"
#include "sfMessages.h"
#include "sfTask.h"

using namespace std;


bool stopTasks = false;

DebugLevel max_debug() { return DebugLevel::DBG_Info; }

SFDebug errdbg(DebugLevel::DBG_Error, max_debug, SFColors::Color_Red, "ErrOut");    // Prints to file
SFDebug wngdbg(DebugLevel::DBG_Warning, max_debug, SFColors::Color_Blue, "stderr"); // Prints to stderr
SFDebug ifodbg(DebugLevel::DBG_Info, max_debug, SFColors::Color_Cyan);              // Prints to stdout
SFDebug vbsdbg(DebugLevel::DBG_Verbose, max_debug, SFColors::Color_White);


int numProcess = 10;
int numMessages = 100;
std::vector<class Task*> processes;

Task* rand_process()
{
	int pNo = (rand() % numProcess);

	return processes[pNo];
}


int main(int argc, char** argv)
{
	// first argument for number of tasks
	if (argc > 1)
	{
		numProcess = atoi(argv[1]);
	}
	// second argument for number of messages to send before quitting	
	if (argc > 2)
	{
		numMessages = atoi(argv[2]);
	}


	srand((int32_t)time(NULL));

	for (int i = 0; i < numProcess; i++)
	{
		processes.push_back(new Task(i, numMessages));
	}

	// Start the first message
	std::string data = "This is my initial message";
	TestMessage sMsg;
	sMsg.msg = data;

	// Send a message to random task
	rand_process()->addMessage(sMsg);

	while(!stopTasks)
	{
		sleep(1);
	}


	for (int i = 0; i < numProcess; i++)
	{
		delete processes[i];
	}
	processes.clear();
}

