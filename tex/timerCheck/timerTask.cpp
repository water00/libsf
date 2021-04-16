#include <iostream>
#include <string>
#include <sstream>
#include "../src/sfThread.h"
#include "../src/sfTimer.h"
#include "timerTask.h"

extern bool toss();
extern bool stopTasks;

extern SFDebug errdbg;
extern SFDebug wngdbg;
extern SFDebug ifodbg;
extern SFDebug vbsdbg;

TimerTask::TimerTask()
{
	// Create sock
	create_sock();

	lastTimerID = 0;
	// Create a struct with all the process fns and 
	// related socket
	ProcessStruct<PROCFN> pStruct;
	pStruct.sock = socks[0];
	pStruct.processFn = &SFTask::processFn;
	pStruct.processObj = this;

	SFTask::sfThread->add_process(pStruct);
}

TimerTask::~TimerTask()
{
}

void TimerTask::processFn()
{
	TimerMessage gMsg;

	while (getMessage(gMsg))
	{
		wngdbg << "Timer ID: " << gMsg.timerID << " Expired" << "\n";
		// If all timers expired, exit out
		if (gMsg.timerID == lastTimerID)
		{
			wngdbg << "Last Timer Expired. Stopping ...." << "\n";
			stopTasks = true;
		}
	}

}

void TimerTask::start_timers()
{
	srand(static_cast<uint32_t>(time(NULL)));

	for (int i = 0; i < 100; i++)
	{
		// Create 0th timer with max time. When this timer expires all other 
		// timers would have completed and this task can exit
		if (i == 0)
		{
			lastTimerID = timers.create_timer(10000, this, false);
		}
		else
		{
			// For testing purpose all of the timers are not continuous
			timers.create_timer(rand() % 10000, this, false);
		}
	}

	timers.print_timers();

}