#include <iostream>
#include <string>
#include "../src/sfTimer.h"
#include "stateTask.h"

bool stopTasks = false;

DebugLevel max_debug() { return DBG_Info; }

SFDebug errdbg(DBG_Error, max_debug, Color_Red, "ErrOut");    // Prints to file
SFDebug wngdbg(DBG_Warning, max_debug, Color_Blue, "stderr"); // Prints to stderr
SFDebug ifodbg(DBG_Info, max_debug, Color_Cyan);              // Prints to stdout
SFDebug vbsdbg(DBG_Verbose, max_debug, Color_White);

void decide(StateTask& stateTask)
{
    char c = (rand() % 2 == 0) ? 's': 'r';

    // Send message to the state task
    // This is sent ever second. Most times it is
    // not relevant as event will be discarded by state machine.
    StateMessage sMsg;
    sMsg.decision = c;
    stateTask.addMessage(sMsg);
}

int main()
{
    srand((int32_t)time(NULL));

    StateTask stateTask;

    while(!stopTasks)
    {
        decide(stateTask);
        sleep(1);
    }

}