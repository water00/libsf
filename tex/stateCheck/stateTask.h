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

struct EventInfo
{
	EVENT event;
	std::any info;
};

class StateTask : public SFTask
{
private:
	typedef void(StateTask::*Action)(const std::any&);
	SFStateMc<STATE, EVENT, Action, StateTask> *stateMc;

    std::recursive_mutex mutex;


	SFTimer timers;
	int32_t timerAId;
	int32_t timerBId;

	std::list<EventInfo> eventInfos;

	CountInfo cInfo;

	void setup_stateMc();

	void process_timer(const TimerMessage& tMsg);
	void process_stateMsg(const StateMessage& sMsg);

	inline void lock() { std::lock_guard<std::recursive_mutex> lock(mutex); }

public:
	StateTask();
	virtual ~StateTask();

	virtual void processFn();

	void start_timers();

	void timerA_first(const std::any& info);
	void timerB_first(const std::any& info);
	void end_state(const std::any& info);
	void restart(const std::any& info);
	void end_prog(const std::any& info);
	bool addEvent(EVENT e, const std::any& info);
	bool getEvent(EventInfo& eInfo);
	void popEvent();
	void stateEvent(EVENT e, const std::any& info);
};
