#include <iostream>
#include <string>
#include "../src/sfTimer.h"
#include "timerTask.h"

bool stopTasks = false;

DebugLevel max_debug() { return DBG_Info; }

SFDebug errdbg(DBG_Error, max_debug, Color_Red, "ErrOut");    // Prints to file
SFDebug wngdbg(DBG_Warning, max_debug, Color_Blue, "stderr"); // Prints to stderr
SFDebug ifodbg(DBG_Info, max_debug, Color_Cyan);              // Prints to stdout
SFDebug vbsdbg(DBG_Verbose, max_debug, Color_White);

bool toss()
{
    return (rand() % 2 == 0) ? false: true;
}

int main()
{
    TimerTask timerTask;

    timerTask.start_timers();


    while(!stopTasks)
    {
        sleep(1);
    }

	SFTask::sfThread->stop_thread();
}