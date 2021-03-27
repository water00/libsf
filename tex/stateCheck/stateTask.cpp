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

void StateTask::process_timer()
{
	TimerMessage gMsg;

	if (getMessage(gMsg))
	{
		wngdbg << "Timer ID: " << gMsg.timerID << " Expired" << "\n";
		if (gMsg.timerID == timerAId)
		{
			stateEvent(EVENT_TimerA);
		}
		else if (gMsg.timerID == timerBId)
		{
			stateEvent(EVENT_TimerB);
		}
	}

}

void StateTask::process_stateMsg()
{
	StateMessage sMsg;

	if (getMessage(sMsg))
	{
		wngdbg << "User decision received : " << sMsg.decision << "\n";
		switch (sMsg.decision)
		{
		case 'r':
			stateEvent(EVENT_Restart);
			break;
		case 's':
			stateEvent(EVENT_Stop);
			break; 
		}
	}
}

void StateTask::processFn()
{
	switch (getCommand())
	{
	case static_cast<int32_t>(SFCommands::INNO_TIMER_EXPIRED):
		process_timer();
		break;
	case static_cast<int32_t>(UserCommands::STATE_DECISION):
		process_stateMsg();
		break;
	}
}

void StateTask::start_timers()
{
    srand(time(NULL));

	// Start 2 timers A and B. Depending on which timer comes first 
	// State will change. Once the other timer also expires, the state
	// machine will complete and end the program
	timerAId = timers.create_timer(1000 + rand() % 10000, this, false);
	timerBId = timers.create_timer(1000 + rand() % 10000, this, false);

    timers.print_timers();
}

void StateTask::timerA_first()
{
	wngdbg << "Timer A received first. Waiting for Timer B" << "\n";
}

void StateTask::timerB_first()
{
	wngdbg << "Timer B received first. Waiting for Timer A" << "\n";
}

void StateTask::end_state()
{
	wngdbg << "Both Timers Received. Waiting for decision..." << "\n";
}

void StateTask::restart()
{
	wngdbg << "Restart received." << "\n";
	timers.enable(timerAId);
	timers.enable(timerBId);
}

void StateTask::end_prog()
{
	wngdbg << "Stop Received. Terminating..." << "\n";

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

void StateTask::stateEvent(EVENT e)
{
    ifodbg << "CurrState: " << stateMc->get_currState()  << ", Event: " << e << "\n"; 
	stateMc->do_actions(this, e);
	ifodbg << "New State: " << stateMc->get_currState() << "\n";
}
