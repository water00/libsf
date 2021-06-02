#include <iostream>
#include <string>
#include <sstream>
#include "../src/sfThread.h"
#include "../src/sfTimer.h"
#include "stateTask.h"

extern bool stopTasks;

extern SFDebug errdbg;
extern SFDebug wngdbg;
extern SFDebug ifodbg;
extern SFDebug vbsdbg;

StateTask::StateTask()
{
	// Create sock
	create_sock();

	// Setup state machine
	setup_stateMc();

	// Create a struct with all the process fns and 
	// related socket
	ProcessStruct<PROCFN> pStruct;
	pStruct.sock = socks[0];
	pStruct.processFn = &SFTask::processFn;
	pStruct.processObj = this;

	SFTask::sfThread->add_process(pStruct);

	// Create Timers
	start_timers();
}

StateTask::~StateTask()
{
	delete stateMc;
}

void StateTask::process_timer(const TimerMessage& tMsg)
{
	wngdbg << "Timer ID: " << tMsg.timerID << " Expired" << "\n";
	if (tMsg.timerID == timerAId)
	{
		cInfo.countA++;
		stateEvent(EVENT_TimerA, cInfo);
	}
	else if (tMsg.timerID == timerBId)
	{
		cInfo.countB++;
		stateEvent(EVENT_TimerB, cInfo);
	}
	else
	{
		errdbg << "Timer ID: " << tMsg.timerID << " is invalid" << "\n";
	}
}

void StateTask::process_stateMsg(const StateMessage& sMsg)
{
	// decisions are auto generated and if they come in at times when 
	// state is not waiting for decision, it would be ignored
	// but stops counts may increase.
	wngdbg << "User decision received : " << sMsg.decision << "\n";
	switch (sMsg.decision)
	{
	case 's':
		cInfo.stopCount++;
		stateEvent(EVENT_Stop, cInfo);
		break; 
	case 'r':
	default:
		cInfo.restartCount++;
		stateEvent(EVENT_Restart, cInfo);
		break;
	}
}

void StateTask::processFn()
{
	SFType msgType;

	if (getMessageType(msgType))
	{
		switch (msgType)
		{
		case SFType::SFTYPE_TIMER:
			{
				TimerMessage tMsg;
				if (getMessage(tMsg))
				{
					process_timer(tMsg);
				}
			}
			break;
		case SFType::SFTYPE_USER:
			{
				StateMessage sMsg;
				if (getMessage(sMsg))
				{
					process_stateMsg(sMsg);
				}
			}
			break;
		}
	}
}

void StateTask::start_timers()
{

	// Start 2 timers A and B. Depending on which timer comes first 
	// State will change. Once the other timer also expires, the state
	// machine will complete and end the program
	timerAId = timers.create_timer(1000 + rand() % 10000, this, false);
	timerBId = timers.create_timer(1000 + rand() % 10000, this, false);

	timers.print_timers();
}

void StateTask::timerA_first(const std::any& info)
{
	wngdbg << "Timer A received first. Waiting for Timer B" << "\n";
}

void StateTask::timerB_first(const std::any& info)
{
	wngdbg << "Timer B received first. Waiting for Timer A" << "\n";
}

void StateTask::end_state(const std::any& info)
{
	wngdbg << "Both Timers Received. Waiting for decision..." << "\n";
}

void StateTask::restart(const std::any& info)
{
	wngdbg << "Restart received." << "\n";
	timers.enable(timerAId);
	timers.enable(timerBId);
}

void StateTask::end_prog(const std::any& info)
{
	wngdbg << "Stop Received. Terminating..." << "\n";
	wngdbg << "Info: countA: " << cInfo.countA << ", countB: " << cInfo.countB << ", restarts: " << cInfo.restartCount << ", stops: " << cInfo.stopCount << "\n";

	stopTasks = true;
}

void StateTask::setup_stateMc()
{
	stateMc = new SFStateMc<STATE, EVENT, Action, StateTask>(STATE_Start);

	StateAction<STATE, Action> sa;

	// actions for STATE_Start, EVENT_TimerA
	sa.actions.clear();
	sa.actions.push_back(&StateTask::timerA_first);
	sa.nextState = STATE_X;
	stateMc->add_eventActions(STATE_Start, EVENT_TimerA, sa);

	// actions for STATE_Start, EVENT_TimerB
	sa.actions.clear();
	sa.actions.push_back(&StateTask::timerB_first);
	sa.nextState = STATE_Y;
	stateMc->add_eventActions(STATE_Start, EVENT_TimerB, sa);

	// actions for STATE_X, EVENT_TimerB
	sa.actions.clear();
	sa.actions.push_back(&StateTask::end_state);
	sa.nextState = STATE_Z;
	stateMc->add_eventActions(STATE_X, EVENT_TimerB, sa);

	// actions for STATE_Y, EVENT_TimerA
	sa.actions.clear();
	sa.actions.push_back(&StateTask::end_state);
	sa.nextState = STATE_Z;
	stateMc->add_eventActions(STATE_Y, EVENT_TimerA, sa);

	// actions for STATE_Z, EVENT_Restart
	sa.actions.clear();
	sa.actions.push_back(&StateTask::restart);
	sa.nextState = STATE_Start;
	stateMc->add_eventActions(STATE_Z, EVENT_Restart, sa);

	// actions for STATE_Y, EVENT_Stop
	sa.actions.clear();
	sa.actions.push_back(&StateTask::end_prog);
	sa.nextState = STATE_Z;
	stateMc->add_eventActions(STATE_Z, EVENT_Stop, sa);

}

bool StateTask::addEvent(EVENT e, const std::any& info)
{
	lock();
	// Save the event for processing
	eventInfos.push_back( {e, info} );
	// If no event being processed, return true for immediate processing
	if (eventInfos.size() == 1)
	{
		return true;
	}
	return false;
}

bool StateTask::getEvent(EventInfo& eInfo)
{
	lock();
	if (eventInfos.size() > 0)
	{
		eInfo = eventInfos.front();
		return true;
	}
	return false;
}

void StateTask::popEvent()
{
	lock();
	eventInfos.pop_front();
}

void StateTask::stateEvent(EVENT e, const std::any& info)
{
	// Add the event and if it has to be processed, start process
	if (addEvent(e, info))
	{
		EventInfo eInfo;
		while (getEvent(eInfo))
		{
			wngdbg << "CurrState: " << stateMc->get_currState()  << ", Event: " << eInfo.event << "\n"; 
			stateMc->do_actions(this, eInfo.event, eInfo.info);
			wngdbg << "New State: " << stateMc->get_currState() << "\n";
			popEvent();
		}
	}
}
