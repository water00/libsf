#pragma once

#include <stdint.h>
#include <string>
#include <list>
#include <any>
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

// Just some info to be passed as argument
struct CountInfo
{
	int32_t countA = 0;		// Num of times timerA expired
	int32_t countB = 0;		// Num of times timerB expired
	int32_t restartCount = 0;
	int32_t stopCount = 0;
};

class StateTask : public SFTask
{
private:
	typedef void(StateTask::*Action)(std::any);
	SFStateMc<STATE, EVENT, Action, StateTask> *stateMc; 

	SFTimer timers;
	int32_t timerAId;
	int32_t timerBId;

	CountInfo cInfo;

	void setup_stateMc();

	void process_timer(const TimerMessage& tMsg);
	void process_stateMsg(const StateMessage& sMsg);
	
public:
	StateTask();
	virtual ~StateTask();

	virtual void processFn();

	void start_timers();

	void timerA_first(std::any info);
	void timerB_first(std::any info);
	void end_state(std::any info);
	void restart(std::any info);
	void end_prog(std::any info);
	void stateEvent(EVENT e, std::any info);
};
