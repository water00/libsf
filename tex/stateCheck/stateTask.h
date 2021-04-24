#pragma once

#include <stdint.h>
#include <string>
#include <list>
#include "../src/sfTask.h"
#include "../src/sfTimer.h"
#include "../src/sfState.h"
#include "userMessages.h"


enum STATE
{
	STATE_Start,
	STATE_X,
	STATE_Y,
	STATE_Z
};

enum EVENT
{
	EVENT_TimerA,	// timer A expired
	EVENT_TimerB,	// timer B expired
	EVENT_Restart,	// Restart received
	EVENT_Stop,		// End prog received
};


class StateTask : public SFTask
{
private:
	typedef void(StateTask::*Action)(void);
	SFStateMc<STATE, EVENT, Action, StateTask> *stateMc; 

    SFTimer timers;
	int32_t timerAId;
	int32_t timerBId;

	void setup_stateMc();

	void process_timer(const TimerMessage& tMsg);
	void process_stateMsg(const StateMessage& sMsg);
	
public:
	StateTask();
	virtual ~StateTask();

	virtual void processFn();

	void start_timers();

	void timerA_first();
	void timerB_first();
	void end_state();
	void restart();
	void end_prog();
	void stateEvent(EVENT e);
};
