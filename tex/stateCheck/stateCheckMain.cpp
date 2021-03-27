#include <iostream>
#include <string>
#include "../src/sfTimer.h"
#include "stateTask.h"

// Declare static fn of sfTask here. 
SFThread<PROCFN>* SFTask::sfThread;
int32_t SFTask::taskCount = 0;

bool stopTasks = false;

DebugLevel max_debug() { return DBG_Info; }

SFDebug errdbg(DBG_Error, max_debug, Color_Red, "ErrOut");    // Prints to file
SFDebug wngdbg(DBG_Warning, max_debug, Color_Blue, "stderr"); // Prints to stderr
SFDebug ifodbg(DBG_Info, max_debug, Color_Cyan);              // Prints to stdout
SFDebug vbsdbg(DBG_Verbose, max_debug, Color_White);

char decide(StateTask& stateTask)
{
	char c = std::getchar();

    // Send message to the state task
    StateMessage sMsg;
    sMsg.decision = c;
    stateTask.addMessage(sMsg);
}

int main()
{
    StateTask stateTask;

    while(!stopTasks)
    {
        decide(stateTask);
        sleep(1);
    }

	SFTask::sfThread->stop_thread();
}