#include <iostream>
#include <string>
#include <sstream>
#include "sfThread.h"
#include "sfTimer.h"
#include "timerTask.h"
#include "sfMessages.h"
#include "sfTask.h"

extern bool toss();
extern bool stopTasks;

extern SFDebug errdbg;
extern SFDebug wngdbg;
extern SFDebug ifodbg;
extern SFDebug vbsdbg;

struct UserData
{
	int32_t i;
	std::string s;
};

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
		UserData uData = std::any_cast<UserData>(gMsg.userData);
		wngdbg << "Timer ID: " << gMsg.timerID << " Expired with userData : { " << uData.i << ", " << uData.s << " }" << "\n";
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
		std::stringstream ss;
		ss << "String_" << i;
		UserData uData {i, ss.str()};

		// Create 0th timer with max time. When this timer expires all other 
		// timers would have completed and this task can exit
		if (i == 0)
		{
			lastTimerID = timers.create_timer(10000, this, false, uData);
		}
		else
		{
			// For testing purpose all of the timers are not continuous
			timers.create_timer(rand() % 10000, this, false, uData);
		}
	}

	timers.print_timers();

}