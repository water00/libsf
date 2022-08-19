#include <iostream>
#include <string>
#include "../../src/sfTimer.h"
#include "timerTask.h"
#include "../../src/sfTask.h"

bool stopTasks = false;

DebugLevel max_debug() { return DebugLevel::DBG_Info; }

SFDebug errdbg(DebugLevel::DBG_Error, max_debug, SFColors::Color_Red, "ErrOut");    // Prints to file
SFDebug wngdbg(DebugLevel::DBG_Warning, max_debug, SFColors::Color_Blue, "stderr"); // Prints to stderr
SFDebug ifodbg(DebugLevel::DBG_Info, max_debug, SFColors::Color_Cyan);              // Prints to stdout
SFDebug vbsdbg(DebugLevel::DBG_Verbose, max_debug, SFColors::Color_White);

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
}