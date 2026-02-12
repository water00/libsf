#pragma once

#include <stdint.h>
#include <string>
#include <list>
#include "sfTask.h"
#include "sfTimer.h"


class TimerTask : public SFTask
{
private:
    SFTimer timers;
	int32_t lastTimerID;

public:
	TimerTask();
	virtual ~TimerTask();

	virtual void processFn();

	void start_timers();
};
